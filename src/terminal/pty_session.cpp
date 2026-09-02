#include <tuinator/terminal/pty_session.hpp>

#ifndef _WIN32

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>

#include <array>
#include <pty.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>

namespace tuinator {

namespace {

winsize make_winsize(Size size) {
    winsize ws{};
    ws.ws_col = static_cast<unsigned short>(std::max(1, size.width));
    ws.ws_row = static_cast<unsigned short>(std::max(1, size.height));
    return ws;
}

void set_winsize(int fd, Size size) {
    const winsize ws = make_winsize(size);
    ioctl(fd, TIOCSWINSZ, &ws);
}

std::string default_shell() {
    if (const char* shell = std::getenv("SHELL")) {
        if (shell[0] != '\0') {
            return shell;
        }
    }
    return "/bin/bash";
}

std::string shell_basename(const std::string& shell) {
    const std::size_t slash = shell.find_last_of('/');
    return slash == std::string::npos ? shell : shell.substr(slash + 1);
}

} // namespace

PtySession::~PtySession() {
    terminate();
}

void PtySession::set_output_handler(OutputHandler handler) {
    std::lock_guard<std::mutex> lock(handler_mutex_);
    output_handler_ = std::move(handler);
}

void PtySession::set_exit_handler(ExitHandler handler) {
    std::lock_guard<std::mutex> lock(handler_mutex_);
    exit_handler_ = std::move(handler);
}

bool PtySession::start(Size size, const std::string& shell_command) {
    terminate();

    const std::string shell = shell_command.empty() ? default_shell() : shell_command;
    const winsize ws = make_winsize(size);

    pid_t pid = forkpty(&master_fd_, nullptr, nullptr, &ws);
    if (pid < 0) {
        master_fd_ = -1;
        return false;
    }

    if (pid == 0) {
        setenv("TERM", "xterm-256color", 1);
        setenv("COLORTERM", "truecolor", 1);

        char lines[16];
        char columns[16];
        std::snprintf(lines, sizeof(lines), "%d", std::max(1, size.height));
        std::snprintf(columns, sizeof(columns), "%d", std::max(1, size.width));
        setenv("LINES", lines, 1);
        setenv("COLUMNS", columns, 1);

        const std::string base = shell_basename(shell);
        execl(shell.c_str(), base.c_str(), "-i", static_cast<char*>(nullptr));
        _exit(127);
    }

    child_pid_ = pid;
    running_ = true;
    shutdown_ = false;

    resize(size);
    start_io_threads();
    return true;
}

void PtySession::resize(Size size) {
    if (master_fd_ >= 0) {
        set_winsize(master_fd_, size);
    }
    if (child_pid_ > 0) {
        kill(child_pid_, SIGWINCH);
    }
}

bool PtySession::enqueue_input(std::string data) {
    if (!running_ || data.empty()) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(write_mutex_);
        write_queue_.push(std::move(data));
    }
    write_cv_.notify_one();
    return true;
}

void PtySession::start_io_threads() {
    if (io_started_ || master_fd_ < 0) {
        return;
    }

    shutdown_ = false;
    reader_thread_ = std::thread([this] { reader_loop(); });
    writer_thread_ = std::thread([this] { writer_loop(); });
    io_started_ = true;
}

void PtySession::stop_io_threads() {
    if (!io_started_) {
        return;
    }

    shutdown_ = true;
    write_cv_.notify_all();

    if (master_fd_ >= 0) {
        close(master_fd_);
        master_fd_ = -1;
    }

    if (reader_thread_.joinable()) {
        reader_thread_.join();
    }
    if (writer_thread_.joinable()) {
        writer_thread_.join();
    }

    {
        std::lock_guard<std::mutex> lock(write_mutex_);
        std::queue<std::string> empty;
        std::swap(write_queue_, empty);
    }

    io_started_ = false;
}

void PtySession::reader_loop() {
    std::array<char, 8192> buffer{};

    while (!shutdown_.load()) {
        const ssize_t bytes = ::read(master_fd_, buffer.data(), buffer.size());
        if (bytes > 0) {
            OutputHandler handler;
            {
                std::lock_guard<std::mutex> lock(handler_mutex_);
                handler = output_handler_;
            }
            if (handler) {
                handler(std::string_view(buffer.data(), static_cast<std::size_t>(bytes)));
            }
            continue;
        }

        if (bytes == 0) {
            running_ = false;
            notify_exit();
            break;
        }

        if (errno == EINTR) {
            continue;
        }

        if (!shutdown_.load()) {
            running_ = false;
            notify_exit();
        }
        break;
    }
}

void PtySession::writer_loop() {
    while (true) {
        std::string data;
        {
            std::unique_lock<std::mutex> lock(write_mutex_);
            write_cv_.wait(lock, [this] {
                return shutdown_.load() || !write_queue_.empty();
            });
            if (shutdown_.load() && write_queue_.empty()) {
                break;
            }
            data = std::move(write_queue_.front());
            write_queue_.pop();
        }

        if (master_fd_ < 0) {
            break;
        }

        std::size_t offset = 0;
        while (offset < data.size()) {
            const ssize_t bytes =
                ::write(master_fd_, data.data() + offset, data.size() - offset);
            if (bytes < 0) {
                if (errno == EINTR) {
                    continue;
                }
                running_ = false;
                notify_exit();
                return;
            }
            offset += static_cast<std::size_t>(bytes);
        }
    }
}

void PtySession::notify_exit() {
    ExitHandler handler;
    {
        std::lock_guard<std::mutex> lock(handler_mutex_);
        handler = exit_handler_;
    }
    if (handler) {
        handler();
    }
}

void PtySession::terminate() {
    stop_io_threads();

    if (child_pid_ > 0) {
        kill(child_pid_, SIGHUP);
        int status = 0;
        waitpid(child_pid_, &status, WNOHANG);
        child_pid_ = -1;
    }

    running_ = false;
}

} // namespace tuinator

#else

namespace tuinator {

PtySession::~PtySession() = default;

void PtySession::set_output_handler(OutputHandler) {}

void PtySession::set_exit_handler(ExitHandler) {}

bool PtySession::start(Size, const std::string&) {
    return false;
}

void PtySession::resize(Size) {}

bool PtySession::enqueue_input(std::string) {
    return false;
}

void PtySession::terminate() {}

} // namespace tuinator

#endif
