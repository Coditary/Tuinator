#pragma once

#include <tuinator/testing/app_driver.hpp>

#include <string>
#include <string_view>

namespace tuinator::testing {

/// Handle one JSON line (newline-delimited protocol for agents). Returns a JSON response object.
std::string handle_json_command(AppDriver& driver, std::string_view line);

/// Run an interactive JSON command loop on stdin/stdout until `quit` or EOF.
int run_json_protocol_loop(AppDriver& driver);

} // namespace tuinator::testing
