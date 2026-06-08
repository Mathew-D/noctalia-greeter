#pragma once

#include "wayland/wayland_client.h"

[[nodiscard]] float uiScaleForOutput(const WaylandOutputInfo &output) noexcept;
