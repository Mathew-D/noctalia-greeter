#include "wayland/display_scale.h"

#include <algorithm>
#include <cmath>

namespace {

constexpr float kTargetDpi = 96.0f;
constexpr float kMinUiScale = 1.0f;
constexpr float kMaxUiScale = 2.0f;
constexpr float k4kFallbackScale = 1.5f;
constexpr int k4kWidth = 3840;
constexpr int k4kHeight = 2160;
constexpr int kHiResWidth = 2560;

[[nodiscard]] float dpiFromAxis(int32_t pixels, int32_t physicalMm) noexcept {
  if (pixels <= 0 || physicalMm <= 0) {
    return 0.0f;
  }
  return static_cast<float>(pixels) / (static_cast<float>(physicalMm) / 25.4f);
}

[[nodiscard]] float effectiveDpi(const WaylandOutputInfo &output) noexcept {
  const float dpiX = dpiFromAxis(output.pixelWidth, output.physicalWidthMm);
  const float dpiY = dpiFromAxis(output.pixelHeight, output.physicalHeightMm);
  if (dpiX > 0.0f && dpiY > 0.0f) {
    return (dpiX + dpiY) * 0.5f;
  }
  if (dpiX > 0.0f) {
    return dpiX;
  }
  if (dpiY > 0.0f) {
    return dpiY;
  }
  return 0.0f;
}

[[nodiscard]] float
fallbackScaleForResolution(const WaylandOutputInfo &output) noexcept {
  if (output.pixelWidth >= k4kWidth || output.pixelHeight >= k4kHeight) {
    return k4kFallbackScale;
  }
  if (output.pixelWidth >= kHiResWidth) {
    return 1.25f;
  }
  return kMinUiScale;
}

} // namespace

float uiScaleForOutput(const WaylandOutputInfo &output) noexcept {
  if (!output.done || output.pixelWidth <= 0 || output.pixelHeight <= 0) {
    return kMinUiScale;
  }

  if (std::max(1, output.scale) > 1) {
    return kMinUiScale;
  }

  const float dpi = effectiveDpi(output);
  if (dpi > kTargetDpi) {
    return std::clamp(dpi / kTargetDpi, kMinUiScale, kMaxUiScale);
  }

  return fallbackScaleForResolution(output);
}
