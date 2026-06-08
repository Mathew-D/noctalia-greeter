#include "ui/style.h"

#include <algorithm>

float Style::s_uiScale = 1.0f;

void Style::setUiScale(float scale) noexcept {
  s_uiScale = std::clamp(scale, kMinUiScale, kMaxUiScale);
}

float Style::uiScale() noexcept { return s_uiScale; }

float Style::scaled(float value) noexcept { return value * s_uiScale; }

float Style::spaceXs() noexcept { return spaceXsBase * s_uiScale; }
float Style::spaceSm() noexcept { return spaceSmBase * s_uiScale; }
float Style::spaceMd() noexcept { return spaceMdBase * s_uiScale; }
float Style::spaceLg() noexcept { return spaceLgBase * s_uiScale; }
float Style::spaceXl() noexcept { return spaceXlBase * s_uiScale; }
float Style::space2xl() noexcept { return space2xlBase * s_uiScale; }
float Style::space3xl() noexcept { return space3xlBase * s_uiScale; }

float Style::controlHeight() noexcept { return controlHeightBase * s_uiScale; }
float Style::controlHeightSm() noexcept {
  return controlHeightSmBase * s_uiScale;
}
float Style::controlHeightLg() noexcept {
  return controlHeightLgBase * s_uiScale;
}

float Style::fontSizeCaption() noexcept {
  return fontSizeCaptionBase * s_uiScale;
}
float Style::fontSizeBody() noexcept { return fontSizeBodyBase * s_uiScale; }
float Style::fontSizeTitle() noexcept { return fontSizeTitleBase * s_uiScale; }
float Style::fontSizeHeading() noexcept {
  return fontSizeHeadingBase * s_uiScale;
}
float Style::fontSizeDisplay() noexcept {
  return fontSizeDisplayBase * s_uiScale;
}

float Style::borderWidth() noexcept { return borderWidthBase * s_uiScale; }

float Style::radiusSm() noexcept { return radiusSmBase * s_uiScale; }
float Style::radiusMd() noexcept { return radiusMdBase * s_uiScale; }
float Style::radiusLg() noexcept { return radiusLgBase * s_uiScale; }
float Style::radiusXl() noexcept { return radiusXlBase * s_uiScale; }

float Style::scaledRadiusSm(float factor) noexcept {
  return radiusSm() * factor;
}
float Style::scaledRadiusMd(float factor) noexcept {
  return radiusMd() * factor;
}
float Style::scaledRadiusLg(float factor) noexcept {
  return radiusLg() * factor;
}
float Style::scaledRadiusXl(float factor) noexcept {
  return radiusXl() * factor;
}
