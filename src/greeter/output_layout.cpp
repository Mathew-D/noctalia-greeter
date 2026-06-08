#include "greeter/output_layout.h"

#include "core/log.h"

#include <array>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace {
constexpr Logger kLog("output-layout");
constexpr int kListAttempts = 3;
constexpr useconds_t kListRetryDelayUs = 20'000;
constexpr useconds_t kSettleDelayUs = 50'000;

std::string g_lastPreferred;
bool g_layoutAttempted = false;
bool g_layoutIsolated = false;

[[nodiscard]] bool runWlrRandr(const std::vector<std::string> &args) {
  if (args.empty()) {
    return true;
  }

  pid_t pid = fork();
  if (pid < 0) {
    return false;
  }
  if (pid == 0) {
    std::vector<const char *> argv;
    argv.reserve(args.size() + 2);
    argv.push_back("wlr-randr");
    for (const std::string &arg : args) {
      argv.push_back(arg.c_str());
    }
    argv.push_back(nullptr);
    execvp("wlr-randr", const_cast<char **>(argv.data()));
    _exit(127);
  }

  int status = 0;
  if (waitpid(pid, &status, 0) < 0) {
    return false;
  }
  return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

[[nodiscard]] std::vector<std::string> listConnectedOutputs() {
  std::vector<std::string> outputs;
  FILE *pipe = popen("wlr-randr 2>/dev/null", "r");
  if (pipe == nullptr) {
    return outputs;
  }

  std::array<char, 512> line{};
  while (fgets(line.data(), static_cast<int>(line.size()), pipe) != nullptr) {
    const std::string_view text(line.data());
    const std::size_t connectedPos = text.find(" connected");
    if (connectedPos == std::string_view::npos) {
      continue;
    }

    std::size_t end = connectedPos;
    while (end > 0 && text[end - 1] == ' ') {
      --end;
    }
    std::size_t start = 0;
    while (start < end && text[start] == ' ') {
      ++start;
    }
    if (end > start) {
      outputs.emplace_back(text.substr(start, end - start));
    }
  }

  pclose(pipe);
  return outputs;
}

[[nodiscard]] std::vector<std::string> listConnectedOutputsWithRetry() {
  for (int attempt = 0; attempt < kListAttempts; ++attempt) {
    std::vector<std::string> outputs = listConnectedOutputs();
    if (!outputs.empty()) {
      return outputs;
    }
    if (attempt + 1 < kListAttempts) {
      usleep(kListRetryDelayUs);
    }
  }
  return {};
}

[[nodiscard]] bool
outputsMatchPreferred(const std::vector<std::string> &outputs,
                      const std::string_view preferredOutput) {
  return outputs.size() == 1 && outputs.front() == preferredOutput;
}

[[nodiscard]] bool applyLayoutOnce(const std::string &preferred,
                                   const std::vector<std::string> &connected) {
  std::vector<std::string> args;
  args.reserve(connected.size() * 4 + 4);
  args.push_back("--output");
  args.push_back(preferred);
  args.push_back("--on");
  args.push_back("--pos");
  args.push_back("0,0");

  for (const std::string &name : connected) {
    if (name == preferred) {
      continue;
    }
    args.push_back("--output");
    args.push_back(name);
    args.push_back("--off");
  }

  if (!runWlrRandr(args)) {
    kLog.warn("wlr-randr layout command failed");
    return false;
  }
  return true;
}

void resetLayoutState(std::string_view preferredOutput) {
  if (g_lastPreferred != preferredOutput) {
    g_lastPreferred.assign(preferredOutput);
    g_layoutAttempted = false;
    g_layoutIsolated = false;
  }
}

} // namespace

namespace greeter {

bool disableNonPreferredOutputs(const std::string_view preferredOutput) {
  if (preferredOutput.empty()) {
    return true;
  }

  resetLayoutState(preferredOutput);
  const std::string preferred(preferredOutput);

  std::vector<std::string> connected = listConnectedOutputsWithRetry();
  if (connected.empty()) {
    kLog.warn("wlr-randr: no connected outputs found");
    return false;
  }

  if (outputsMatchPreferred(connected, preferredOutput)) {
    g_layoutAttempted = true;
    g_layoutIsolated = true;
    return true;
  }

  if (g_layoutAttempted) {
    return g_layoutIsolated;
  }

  g_layoutAttempted = true;

  if (!applyLayoutOnce(preferred, connected)) {
    kLog.warn("wlr-randr could not isolate '{}'; viewport fallback may be used",
              preferredOutput);
    return false;
  }

  usleep(kSettleDelayUs);
  connected = listConnectedOutputs();
  if (outputsMatchPreferred(connected, preferredOutput)) {
    g_layoutIsolated = true;
    kLog.info("single-output layout active on '{}'", preferredOutput);
    return true;
  }

  kLog.warn("wlr-randr could not isolate '{}'; viewport fallback may be used",
            preferredOutput);
  return false;
}

} // namespace greeter
