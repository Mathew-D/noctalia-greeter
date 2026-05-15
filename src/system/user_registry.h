#pragma once

#include <sys/types.h>

#include <string>
#include <vector>

namespace noctalia::system {

struct User {
  std::string name;
  std::string realName;
  std::string home;
  std::string shell;
  uid_t uid = 0;
  gid_t gid = 0;
};

std::vector<User> enumerateUsers();

} // namespace noctalia::system

