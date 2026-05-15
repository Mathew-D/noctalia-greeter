#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace noctalia::session {

enum class SessionType { X11, Wayland };

struct Session {
  std::string id;
  std::string name;
  std::string exec;
  std::filesystem::path desktopFile;
  SessionType type = SessionType::Wayland;
};

std::vector<Session> discoverSessions();

} // namespace noctalia::session

