#pragma once

#include "render/shader_program.h"
#include "render/text_renderer.h"

#include <string>
#include <vector>

namespace noctalia::render {

struct Color {
  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
  float a = 1.0f;
};

class GlRenderer {
public:
  void initialize();
  void beginFrame(int width, int height);
  void drawRect(float x, float y, float width, float height, float radius, Color color, Color border = {},
                float borderWidth = 0.0f);
  void drawTextBlocks(float x, float y, const std::vector<std::string>& lines, Color color);
  void drawText(float x, float y, const std::string& text, float maxWidth, int fontSize, Color color);
  void drawGlyph(float x, float y, std::string_view name, int fontSize, Color color);

private:
  ShaderProgram m_rectProgram;
  ShaderProgram m_textProgram;
  TextRenderer m_textRenderer;
  GLuint m_quadBuffer = 0;
  int m_width = 1;
  int m_height = 1;
};

} // namespace noctalia::render
