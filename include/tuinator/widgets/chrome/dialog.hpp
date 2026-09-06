#pragma once

#include <tuinator/render/theme.hpp>
#include <tuinator/widgets/widget.hpp>

#include <functional>
#include <memory>
#include <string>

namespace tuinator::dialog {

std::unique_ptr<Widget> make_message(const Theme& theme, std::string message, std::function<void()> on_ok = {});

std::unique_ptr<Widget> make_confirm(const Theme& theme, std::string message,
                                     std::function<void(bool confirmed)> on_result);

} // namespace tuinator::dialog
