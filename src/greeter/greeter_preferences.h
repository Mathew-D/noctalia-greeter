#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace greeter {

struct GreeterPreferences {
  std::optional<std::string> session;
  std::optional<std::string> scheme;
};

[[nodiscard]] std::filesystem::path greeterConfPath();

[[nodiscard]] GreeterPreferences loadGreeterPreferences();
[[nodiscard]] bool saveGreeterPreferences(const GreeterPreferences &prefs);

// Root only: synced data dir, greeter.conf, chown conf to greeterUser.
[[nodiscard]] bool installGreeterSystemLayout(std::string_view greeterUser,
                                              std::string &errorOut);

} // namespace greeter
