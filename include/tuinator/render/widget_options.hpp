#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace tuinator {

/// Resolved widget configuration values from a stylesheet (layout, behavior, etc.).
class WidgetOptions {
  public:
    void set(std::string_view key, std::string_view value);

    bool empty() const { return values_.empty(); }
    bool has(std::string_view key) const;

    bool bool_or(std::string_view key, bool fallback) const;
    int int_or(std::string_view key, int fallback) const;
    std::string string_or(std::string_view key, std::string_view fallback) const;

    template <typename Fn>
    void for_each(Fn&& visitor) const {
        for (const auto& entry : values_) {
            visitor(entry.first, entry.second);
        }
    }

  private:
    friend class Stylesheet;
    friend void merge_widget_options(WidgetOptions& target, const WidgetOptions& source);

    std::unordered_map<std::string, std::string> values_;
};

void merge_widget_options(WidgetOptions& target, const WidgetOptions& source);

} // namespace tuinator
