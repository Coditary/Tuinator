#pragma once

#include <tuinator/core/geometry.hpp>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace tuinator {

/// Host-side pseudo-terminal session (POSIX forkpty).
///
/// Spawns dedicated reader/writer threads (tui-term pattern): the reader
/// continuously drains PTY output; input is queued to the writer thread.
class PtySession {
public:
    using OutputHandler = std::function<void(std::string_view)>;
    using ExitHandler = std::function<void()>;

    PtySession() = default;
    ~PtySession();

    PtySession(const PtySession&) = delete;
    PtySession& operator=(const PtySession&) = delete;

    void set_output_handler(OutputHandler handler);
    void set_exit_handler(ExitHandler handler);

    bool start(Size size, const std::string& shell_command = {});
    bool is_running() const { return running_; }

    void resize(Size size);
    bool enqueue_input(std::string data);

    void terminate();

private:
    void start_io_threads();
    void stop_io_threads();
    void reader_loop();
    void writer_loop();
    void notify_exit();

    OutputHandler output_handler_;
    ExitHandler exit_handler_;
    std::mutex handler_mutex_;

    int master_fd_ = -1;
    int child_pid_ = -1;
    bool running_ = false;

    std::atomic<bool> shutdown_{false};
    bool io_started_ = false;
    std::thread reader_thread_;
    std::thread writer_thread_;

    std::mutex write_mutex_;
    std::condition_variable write_cv_;
    std::queue<std::string> write_queue_;
};

} // namespace tuinator
