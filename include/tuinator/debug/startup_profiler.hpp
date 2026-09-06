#pragma once

#include <string>
#include <tuple>
#include <vector>

namespace tuinator {

class StartupProfiler {
  public:
    static StartupProfiler& instance();

    void reset();
    void mark(std::string label);
    bool enabled() const { return enabled_; }
    void set_enabled(bool enabled);

    void write_report(const std::string& path) const;

  private:
    StartupProfiler() = default;

    bool enabled_ = false;
    std::size_t start_ns_ = 0;
    std::size_t last_ns_ = 0;
    std::vector<std::tuple<std::string, double, double>> entries_;
};

void startup_profile_mark(std::string label);

} // namespace tuinator
