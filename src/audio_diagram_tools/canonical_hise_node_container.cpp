#include "audio_diagram_tools/canonical_renderers.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <string>

#include <embedded/fonts.h>

namespace adt::canonical::renderers {
namespace {

constexpr float kReferenceWidth = 500.0f;
constexpr float kReferenceHeight = 357.0f;

uint8_t colorChannel(uint32_t color, int shift) {
  return static_cast<uint8_t>((color >> shift) & 0xffu);
}

uint32_t lerpColor(uint32_t a, uint32_t b, float t) {
  t = drawing::clamp01(t);
  const auto lerp = [t](uint8_t from, uint8_t to) {
    return static_cast<uint8_t>(std::lround(static_cast<float>(from) +
                                           (static_cast<float>(to) - static_cast<float>(from)) * t));
  };

  return (static_cast<uint32_t>(lerp(colorChannel(a, 24), colorChannel(b, 24))) << 24) |
         (static_cast<uint32_t>(lerp(colorChannel(a, 16), colorChannel(b, 16))) << 16) |
         (static_cast<uint32_t>(lerp(colorChannel(a, 8), colorChannel(b, 8))) << 8) |
         static_cast<uint32_t>(lerp(colorChannel(a, 0), colorChannel(b, 0)));
}

visage::Font hiseFont(float size) {
  const std::filesystem::path arial_rounded = "C:/Windows/Fonts/ARLRDBD.TTF";
  if (std::filesystem::exists(arial_rounded))
    return visage::Font(size, arial_rounded.string());

  return visage::Font(size, visage::fonts::Lato_Regular_ttf);
}

void drawRect(visage::Canvas& canvas,
              float x,
              float y,
              float width,
              float height,
              uint32_t fill,
              uint32_t stroke,
              float stroke_width) {
  canvas.setColor(fill);
  canvas.rectangle(x, y, width, height);
  canvas.setColor(stroke);
  canvas.rectangleBorder(x, y, width, height, stroke_width);
}

void drawTopWeightedGradientRect(visage::Canvas& canvas,
                                 float x,
                                 float y,
                                 float width,
                                 float height,
                                 uint32_t top,
                                 uint32_t bottom,
                                 uint32_t stroke,
                                 float stroke_width) {
  const int strips = std::max(1, static_cast<int>(std::ceil(height)));
  for (int strip = 0; strip < strips; ++strip) {
    const float t = static_cast<float>(strip) / static_cast<float>(std::max(1, strips - 1));
    const float weighted = std::pow(t, 0.36f);
    canvas.setColor(lerpColor(top, bottom, weighted));
    canvas.rectangle(x, y + static_cast<float>(strip), width, 1.25f);
  }
  canvas.setColor(stroke);
  canvas.rectangleBorder(x, y, width, height, stroke_width);
}

void drawPowerIcon(visage::Canvas& canvas,
                   float center_x,
                   float center_y,
                   float scale,
                   bool power_on) {
  const uint32_t color = power_on ? 0xff7acd93 : 0xff6b6b6b;
  const float radius = 11.5f * scale;
  const float width = 5.0f * scale;

  canvas.setColor(color);
  canvas.ring(center_x - radius, center_y - radius, radius * 2.0f, width);

  canvas.setColor(0xff464646);
  canvas.rectangle(center_x - 5.5f * scale, center_y - 14.5f * scale,
                   11.0f * scale, 12.0f * scale);

  drawing::drawLine(canvas,
                    center_x,
                    center_y - 9.4f * scale,
                    center_x,
                    center_y - 1.2f * scale,
                    width,
                    color);
}

void drawCloseButton(visage::Canvas& canvas, float center_x, float center_y, float scale) {
  constexpr uint32_t kCloseColor = 0xff9d9d9d;
  const float arm = 9.0f * scale;
  drawing::drawLine(canvas,
                    center_x - arm,
                    center_y - arm,
                    center_x + arm,
                    center_y + arm,
                    6.4f * scale,
                    kCloseColor);
  drawing::drawLine(canvas,
                    center_x + arm,
                    center_y - arm,
                    center_x - arm,
                    center_y + arm,
                    6.4f * scale,
                    kCloseColor);
}

} // namespace

void drawHiseNodeContainer(drawing::DrawContext& context,
                           const Dimensions& dimensions,
                           const HiseNodeContainerOptions& options) {
  const float scale = std::min(static_cast<float>(dimensions.width) / kReferenceWidth,
                               static_cast<float>(dimensions.height) / kReferenceHeight);
  const float ox = (static_cast<float>(dimensions.width) - kReferenceWidth * scale) * 0.5f;
  const float oy = (static_cast<float>(dimensions.height) - kReferenceHeight * scale) * 0.5f;

  drawHiseNodeContainerAt(context, options, ox, oy, scale);
}

void drawHiseNodeContainerAt(drawing::DrawContext& context,
                             const HiseNodeContainerOptions& options,
                             float origin_x,
                             float origin_y,
                             float scale) {
  visage::Canvas& canvas = context.canvas;

  auto sx = [&](float value) { return origin_x + value * scale; };
  auto sy = [&](float value) { return origin_y + value * scale; };
  auto sw = [&](float value) { return value * scale; };

  drawRect(canvas, sx(2.0f), sy(2.0f), sw(496.0f), sw(351.0f),
           0xff464646, 0xff555555, sw(2.0f));

  drawTopWeightedGradientRect(canvas,
                              sx(6.0f),
                              sy(49.0f),
                              sw(488.0f),
                              sw(302.0f),
                              0xff2c2c2c,
                              0xff353535,
                              0xff242424,
                              sw(2.0f));

  drawRect(canvas, sx(82.0f), sy(84.0f), sw(334.0f), sw(231.0f),
           0xff262626, 0xff060609, sw(3.0f));

  drawPowerIcon(canvas, sx(26.0f), sy(25.0f), scale, options.power_on);
  if (options.draw_close_button)
    drawCloseButton(canvas, sx(476.0f), sy(25.5f), scale);

  canvas.setColor(0xffebebeb);
  canvas.text(options.label.c_str(),
              hiseFont(90.0f * scale),
              visage::Font::kCenter,
              sx(82.0f),
              sy(83.0f),
              sw(334.0f),
              sw(232.0f));
}

} // namespace adt::canonical::renderers
