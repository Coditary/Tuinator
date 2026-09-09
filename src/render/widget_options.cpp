#include <tuinator/render/widget_options.hpp>

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace tuinator {

namespace {

std::string normalize_key(std::string_view key) {
    std::string normalized;
    normalized.reserve(key.size());
    for (char ch : key) {
        if (ch == '_') {
            normalized.push_back('-');
            continue;
        }
        normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
    }
    return normalized;
}

bool parse_bool(std::string_view value) {
    if (value == "1" || value == "true" || value == "yes" || value == "on") {
        return true;
    }
    if (value == "0" || value == "false" || value == "no" || value == "off") {
        return false;
    }
    throw std::runtime_error("Expected boolean widget option value, got: " + std::string(value));
}

} // namespace

void WidgetOptions::set(std::string_view key, std::string_view value) {
    values_[normalize_key(key)] = std::string(value);
}

bool WidgetOptions::has(std::string_view key) const {
    return values_.find(normalize_key(key)) != values_.end();
}

bool WidgetOptions::bool_or(std::string_view key, bool fallback) const {
    const auto it = values_.find(normalize_key(key));
    if (it == values_.end()) {
        return fallback;
    }
    return parse_bool(it->second);
}

int WidgetOptions::int_or(std::string_view key, int fallback) const {
    const auto it = values_.find(normalize_key(key));
    if (it == values_.end()) {
        return fallback;
    }

    try {
        std::size_t consumed = 0;
        const int value = std::stoi(it->second, &consumed);
        if (consumed != it->second.size()) {
            throw std::invalid_argument("trailing");
        }
        return value;
    } catch (const std::exception&) {
        throw std::runtime_error("Expected integer widget option for '" + std::string(key) + "', got: " + it->second);
    }
}

std::string WidgetOptions::string_or(std::string_view key, std::string_view fallback) const {
    const auto it = values_.find(normalize_key(key));
    if (it == values_.end()) {
        return std::string(fallback);
    }
    return it->second;
}

void merge_widget_options(WidgetOptions& target, const WidgetOptions& source) {
    for (const auto& [key, value] : source.values_) {
        target.values_[key] = value;
    }
}

} // namespace tuinator
