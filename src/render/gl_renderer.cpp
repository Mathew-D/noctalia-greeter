#include "render/gl_renderer.h"

#include "render/glyph_registry.h"

#include <GLES2/gl2.h>

#include <array>
#include <stdexcept>

namespace noctalia::render {
namespace {
constexpr char kRectVertexShader[] = R"(
precision highp float;
attribute vec2 a_position;
uniform vec2 u_surface_size;
uniform vec4 u_rect;
varying vec2 v_pixel;
vec2 to_ndc(vec2 pixel_pos) {
    vec2 normalized = pixel_pos / u_surface_size;
    return vec2(normalized.x * 2.0 - 1.0, 1.0 - normalized.y * 2.0);
}
void main() {
    vec2 local = a_position * u_rect.zw;
    v_pixel = local;
    gl_Position = vec4(to_ndc(u_rect.xy + local), 0.0, 1.0);
}
)";

constexpr char kRectFragmentShader[] = R"(
#ifdef GL_OES_standard_derivatives
#extension GL_OES_standard_derivatives : enable
#endif
precision highp float;
uniform vec4 u_color;
uniform vec4 u_border_color;
uniform vec2 u_rect_size;
uniform float u_radius;
uniform float u_border_width;
varying vec2 v_pixel;
float rounded_rect_distance(vec2 point, vec2 size, float radius) {
    vec2 half_size = size * 0.5;
    vec2 centered = point - half_size;
    float r = min(radius, min(half_size.x, half_size.y));
    vec2 q = abs(centered) - (half_size - vec2(r));
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}
void main() {
    float d = rounded_rect_distance(v_pixel, u_rect_size, u_radius);
#ifdef GL_OES_standard_derivatives
    float aa = max(fwidth(d), 0.75);
#else
    float aa = 1.0;
#endif
    float alpha = 1.0 - smoothstep(-aa, aa, d);
    vec4 fill = u_color;
    if (u_border_width > 0.0) {
        float inner = d + u_border_width;
        float mix_border = 1.0 - smoothstep(-aa, aa, inner);
        fill = mix(u_border_color, u_color, mix_border);
    }
    gl_FragColor = vec4(fill.rgb, fill.a * alpha);
}
)";

constexpr char kTextVertexShader[] = R"(
precision highp float;
attribute vec2 a_position;
uniform vec2 u_surface_size;
uniform vec4 u_rect;
varying vec2 v_texcoord;
vec2 to_ndc(vec2 pixel_pos) {
    vec2 normalized = pixel_pos / u_surface_size;
    return vec2(normalized.x * 2.0 - 1.0, 1.0 - normalized.y * 2.0);
}
void main() {
    v_texcoord = a_position;
    gl_Position = vec4(to_ndc(u_rect.xy + a_position * u_rect.zw), 0.0, 1.0);
}
)";

constexpr char kTextFragmentShader[] = R"(
precision mediump float;
uniform vec4 u_color;
uniform sampler2D u_texture;
varying vec2 v_texcoord;
void main() {
    float alpha = texture2D(u_texture, v_texcoord).a;
    gl_FragColor = vec4(u_color.rgb * u_color.a * alpha, u_color.a * alpha);
}
)";
} // namespace

void GlRenderer::initialize() {
  m_rectProgram.create(kRectVertexShader, kRectFragmentShader);
  m_textProgram.create(kTextVertexShader, kTextFragmentShader);
  const std::array<float, 8> quad{0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
  glGenBuffers(1, &m_quadBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, m_quadBuffer);
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(quad.size() * sizeof(float)), quad.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GlRenderer::beginFrame(int width, int height) {
  m_width = width > 0 ? width : 1;
  m_height = height > 0 ? height : 1;
  glViewport(0, 0, m_width, m_height);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_SCISSOR_TEST);
  glClearColor(0.035f, 0.036f, 0.045f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
}

void GlRenderer::drawRect(float x, float y, float width, float height, float radius, Color color, Color border,
                          float borderWidth) {
  glUseProgram(m_rectProgram.id());
  glBindBuffer(GL_ARRAY_BUFFER, m_quadBuffer);
  const GLint pos = glGetAttribLocation(m_rectProgram.id(), "a_position");
  glEnableVertexAttribArray(static_cast<GLuint>(pos));
  glVertexAttribPointer(static_cast<GLuint>(pos), 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  glUniform2f(glGetUniformLocation(m_rectProgram.id(), "u_surface_size"), static_cast<float>(m_width),
              static_cast<float>(m_height));
  glUniform4f(glGetUniformLocation(m_rectProgram.id(), "u_rect"), x, y, width, height);
  glUniform2f(glGetUniformLocation(m_rectProgram.id(), "u_rect_size"), width, height);
  glUniform1f(glGetUniformLocation(m_rectProgram.id(), "u_radius"), radius);
  glUniform1f(glGetUniformLocation(m_rectProgram.id(), "u_border_width"), borderWidth);
  glUniform4f(glGetUniformLocation(m_rectProgram.id(), "u_color"), color.r * color.a, color.g * color.a,
              color.b * color.a, color.a);
  glUniform4f(glGetUniformLocation(m_rectProgram.id(), "u_border_color"), border.r * border.a, border.g * border.a,
              border.b * border.a, border.a);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void GlRenderer::drawTextBlocks(float x, float y, const std::vector<std::string>& lines, Color color) {
  std::string text;
  for (std::size_t i = 0; i < lines.size(); ++i) {
    if (i > 0) {
      text += '\n';
    }
    text += lines[i];
  }
  drawText(x, y, text, static_cast<float>(m_width) - x - 32.0f, 15, color);
}

void GlRenderer::drawText(float x, float y, const std::string& text, float maxWidth, int fontSize, Color color) {
  TextTexture texture = m_textRenderer.render(text, fontSize, static_cast<int>(maxWidth));
  glUseProgram(m_textProgram.id());
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture.texture);
  glBindBuffer(GL_ARRAY_BUFFER, m_quadBuffer);
  const GLint pos = glGetAttribLocation(m_textProgram.id(), "a_position");
  glEnableVertexAttribArray(static_cast<GLuint>(pos));
  glVertexAttribPointer(static_cast<GLuint>(pos), 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  glUniform2f(glGetUniformLocation(m_textProgram.id(), "u_surface_size"), static_cast<float>(m_width),
              static_cast<float>(m_height));
  glUniform4f(glGetUniformLocation(m_textProgram.id(), "u_rect"), x, y, static_cast<float>(texture.width),
              static_cast<float>(texture.height));
  glUniform4f(glGetUniformLocation(m_textProgram.id(), "u_color"), color.r, color.g, color.b, color.a);
  glUniform1i(glGetUniformLocation(m_textProgram.id(), "u_texture"), 0);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  m_textRenderer.destroy(texture);
}

void GlRenderer::drawGlyph(float x, float y, std::string_view name, int fontSize, Color color) {
  TextTexture texture =
      m_textRenderer.renderWithFont(utf8(glyphCodepoint(name)), "noctalia-tabler-icons", fontSize, fontSize + 8);
  glUseProgram(m_textProgram.id());
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture.texture);
  glBindBuffer(GL_ARRAY_BUFFER, m_quadBuffer);
  const GLint pos = glGetAttribLocation(m_textProgram.id(), "a_position");
  glEnableVertexAttribArray(static_cast<GLuint>(pos));
  glVertexAttribPointer(static_cast<GLuint>(pos), 2, GL_FLOAT, GL_FALSE, 0, nullptr);
  glUniform2f(glGetUniformLocation(m_textProgram.id(), "u_surface_size"), static_cast<float>(m_width),
              static_cast<float>(m_height));
  glUniform4f(glGetUniformLocation(m_textProgram.id(), "u_rect"), x, y, static_cast<float>(texture.width),
              static_cast<float>(texture.height));
  glUniform4f(glGetUniformLocation(m_textProgram.id(), "u_color"), color.r, color.g, color.b, color.a);
  glUniform1i(glGetUniformLocation(m_textProgram.id(), "u_texture"), 0);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  m_textRenderer.destroy(texture);
}

} // namespace noctalia::render
