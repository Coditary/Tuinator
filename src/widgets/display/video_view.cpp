#include <tuinator/widgets/display/video_view.hpp>

#include <tuinator/render/paint_context.hpp>

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace tuinator {

VideoView::VideoView(VideoViewOptions options) : options_(options) {}

VideoView::VideoView(std::string path, VideoViewOptions options) : options_(options) {
    open(std::move(path));
}

VideoView::~VideoView() { close(); }

void VideoView::layout(Rect bounds) {
    const bool size_changed = bounds.width != bounds_.width || bounds.height != bounds_.height;
    bounds_ = bounds;
    if (size_changed && pid_ > 0) {
        // Re-spawn so the ffmpeg scale filter matches the new cell area.
        const std::string path = path_;
        close();
        open(path);
    }
}

bool VideoView::open(const std::string& path) {
    close();
    path_ = path;

    Size cells = options_.display_cells;
    if (cells.width <= 0 || cells.height <= 0) {
        cells = {std::max(8, bounds_.width), std::max(4, bounds_.height)};
    }
    // ~8x16 px per cell, capped for throughput.
    pixel_width_ = std::min(options_.max_pixel_size.width, std::max(8, cells.width * 8));
    pixel_height_ = std::min(options_.max_pixel_size.height, std::max(8, cells.height * 16));
    pixel_width_ -= pixel_width_ % 2; // even sizes keep ffmpeg happy
    pixel_height_ -= pixel_height_ % 2;
    frame_bytes_ = static_cast<std::size_t>(pixel_width_ * pixel_height_ * 4);
    read_buf_.clear();
    read_buf_.reserve(frame_bytes_ * 2);
    current_ = {};
    frames_shown_ = 0;

    return spawn();
}

void VideoView::close() {
    if (pipe_fd_ >= 0) {
        ::close(pipe_fd_);
        pipe_fd_ = -1;
    }
    if (pid_ > 0) {
        ::kill(pid_, SIGTERM);
        int status = 0;
        ::waitpid(pid_, &status, 0);
        pid_ = -1;
    }
    draining_ = false;
}

bool VideoView::spawn() {
    int fds[2];
    if (::pipe(fds) != 0) {
        return false;
    }

    const std::string scale = "scale=" + std::to_string(pixel_width_) + ":" + std::to_string(pixel_height_);
    const std::string fps = std::to_string(options_.fps);

    const pid_t pid = ::fork();
    if (pid < 0) {
        ::close(fds[0]);
        ::close(fds[1]);
        return false;
    }
    if (pid == 0) {
        ::dup2(fds[1], STDOUT_FILENO);
        const int devnull = ::open("/dev/null", O_WRONLY);
        if (devnull >= 0) {
            ::dup2(devnull, STDERR_FILENO);
        }
        ::close(fds[0]);
        ::close(fds[1]);
        ::execlp("ffmpeg", "ffmpeg", "-loglevel", "error", "-nostdin", "-i", path_.c_str(), "-vf",
                 scale.c_str(), "-r", fps.c_str(), "-f", "rawvideo", "-pix_fmt", "rgba", "-",
                 static_cast<char*>(nullptr));
        ::_exit(127);
    }

    ::close(fds[1]);
    ::fcntl(fds[0], F_SETFL, O_NONBLOCK);
    pipe_fd_ = fds[0];
    pid_ = pid;
    started_ = std::chrono::steady_clock::now();
    frames_shown_ = 0; // pacing restarts with each (re)spawn, e.g. on loop
    return true;
}

void VideoView::pump_pipe() {
    if (pipe_fd_ < 0 || frame_bytes_ == 0) {
        return;
    }
    // Keep at most two decoded frames in userspace. Once we stop reading, the
    // kernel pipe fills up and ffmpeg blocks on write — that backpressure is
    // what throttles the decoder to playback speed.
    const std::size_t capacity = frame_bytes_ * 2;
    std::uint8_t chunk[16384];
    while (read_buf_.size() < capacity) {
        const ssize_t n = ::read(pipe_fd_, chunk, sizeof(chunk));
        if (n > 0) {
            read_buf_.insert(read_buf_.end(), chunk, chunk + n);
            continue;
        }
        if (n == 0) { // EOF: decoder finished; drain the buffer before looping
            int status = 0;
            ::waitpid(pid_, &status, 0); // stdout closed: child is exiting
            ::close(pipe_fd_);
            pipe_fd_ = -1;
            pid_ = -1;
            draining_ = true;
            return;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }
        return; // read error: keep the process, retry next idle
    }
}

void VideoView::advance_frame() {
    if (frame_bytes_ == 0) {
        return;
    }
    if (draining_ && read_buf_.size() < frame_bytes_) {
        // Final buffered frames of the pass have been shown: loop or stop.
        draining_ = false;
        read_buf_.clear();
        if (options_.loop) {
            spawn();
        }
        return;
    }
    const auto elapsed = std::chrono::steady_clock::now() - started_;
    const std::size_t due =
        static_cast<std::size_t>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() *
                                 options_.fps / 1000);
    if (due <= frames_shown_) {
        return; // not time for the next frame yet
    }
    const std::size_t available = read_buf_.size() / frame_bytes_;
    if (available == 0) {
        return; // decoder behind: hold current frame
    }
    // Drop frames when behind schedule so playback stays real-time.
    const std::size_t wanted = due - frames_shown_;
    const std::size_t take = std::min(wanted, available);
    const std::size_t offset = (take - 1) * frame_bytes_;
    current_ = TerminalImage(pixel_width_, pixel_height_,
                             std::vector<std::uint8_t>(read_buf_.begin() + static_cast<std::ptrdiff_t>(offset),
                                                       read_buf_.begin() + static_cast<std::ptrdiff_t>(offset + frame_bytes_)));
    read_buf_.erase(read_buf_.begin(), read_buf_.begin() + static_cast<std::ptrdiff_t>(take * frame_bytes_));
    frames_shown_ += take;
    mark_dirty();
}

void VideoView::on_idle() {
    pump_pipe();
    advance_frame();
}

void VideoView::paint(PaintContext& ctx) const {
    if (bounds_.width <= 0 || bounds_.height <= 0) {
        return;
    }
    paint_bounds_background(ctx);
    if (current_.empty()) {
        return;
    }
    const int cols = std::min(bounds_.width, std::max(4, pixel_width_ / 8));
    const int rows = std::min(bounds_.height, std::max(2, pixel_height_ / 16));
    ctx.canvas.draw_image({0, 0}, {cols, rows}, current_);
}

} // namespace tuinator
