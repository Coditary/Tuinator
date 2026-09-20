#include <tuinator/platform/clipboard.hpp>

#include <cstdio>
#include <cstdlib>
#include <string>

namespace tuinator::clipboard {

namespace {

std::string g_buffer;

bool write_to_command(const char* command, std::string_view text) {
    FILE* pipe = popen(command, "w");
    if (pipe == nullptr) {
        return false;
    }

    const std::size_t written = std::fwrite(text.data(), 1, text.size(), pipe);
    const int status = pclose(pipe);
    return status == 0 && written == text.size();
}

bool read_from_command(const char* command, std::string& out) {
    FILE* pipe = popen(command, "r");
    if (pipe == nullptr) {
        return false;
    }

    out.clear();
    char chunk[4096];
    while (std::fgets(chunk, sizeof(chunk), pipe) != nullptr) {
        out.append(chunk);
    }

    const int status = pclose(pipe);
    if (status != 0) {
        out.clear();
        return false;
    }

    while (!out.empty() && (out.back() == '\n' || out.back() == '\r')) {
        out.pop_back();
    }

    return true;
}

void sync_to_os(std::string_view text) {
    if (std::getenv("WAYLAND_DISPLAY") != nullptr) {
        if (write_to_command("wl-copy --type text/plain 2>/dev/null", text)) {
            return;
        }
    }

    if (std::getenv("DISPLAY") != nullptr) {
        write_to_command("xclip -selection clipboard 2>/dev/null", text);
    }
}

std::string read_from_os() {
    std::string external;
    if (std::getenv("WAYLAND_DISPLAY") != nullptr) {
        if (read_from_command("wl-paste --type text/plain --no-newline 2>/dev/null", external)) {
            return external;
        }
        if (read_from_command("wl-paste --type text/plain 2>/dev/null", external)) {
            return external;
        }
    }

    if (std::getenv("DISPLAY") != nullptr) {
        if (read_from_command("xclip -selection clipboard -o 2>/dev/null", external)) {
            return external;
        }
    }

    return {};
}

} // namespace

void set(std::string_view text) {
    g_buffer.assign(text.data(), text.size());
    sync_to_os(text);
}

std::string get() {
    const std::string external = read_from_os();
    if (!external.empty()) {
        g_buffer = external;
        return external;
    }
    return g_buffer;
}

} // namespace tuinator::clipboard
