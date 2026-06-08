#pragma once

#include <string_view>

namespace greeter {

[[nodiscard]] bool disableNonPreferredOutputs(std::string_view preferredOutput);

} // namespace greeter
