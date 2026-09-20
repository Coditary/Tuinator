#pragma once

#include <string>
#include <string_view>

namespace tuinator::clipboard {

/// Store text in the in-app buffer and, when available, the OS clipboard.
void set(std::string_view text);

/// Read text from the OS clipboard when possible, otherwise the in-app buffer.
std::string get();

} // namespace tuinator::clipboard
