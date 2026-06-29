#include "audio_diagram_tools/canonical_renderers.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace adt::canonical::renderers {
using namespace drawing;

void drawDoubleArrowGraphic(visage::Canvas& canvas, const Dimensions& dimensions) {
  constexpr float kReferenceWidth = 346.0f;
  constexpr float kReferenceHeight = 270.0f;
  constexpr float kStrokeWidth = 16.6f;

  const float scale = std::min(static_cast<float>(dimensions.width) / kReferenceWidth,
                               static_cast<float>(dimensions.height) / kReferenceHeight);
  const float origin_x = (static_cast<float>(dimensions.width) - kReferenceWidth * scale) * 0.5f;
  const float origin_y =
      (static_cast<float>(dimensions.height) - kReferenceHeight * scale) * 0.5f;
  auto sx = [&](float x) { return origin_x + x * scale; };
  auto sy = [&](float y) { return origin_y + y * scale; };

  canvas.setColor(0xffffffff);
  canvas.fill(0, 0, dimensions.width, dimensions.height);

  drawChevron(canvas,
              sx(127.0f),
              sy(79.0f),
              sx(168.0f),
              sy(123.0f),
              sx(209.0f),
              kStrokeWidth * scale,
              0xffa1a7c7);
  drawChevron(canvas,
              sx(127.0f),
              sy(144.0f),
              sx(168.0f),
              sy(188.0f),
              sx(209.0f),
              kStrokeWidth * scale,
              0xff737fa7);
}

void drawDoubleArrowGraphicWithColor(visage::Canvas& canvas,
                                     const Dimensions& dimensions,
                                     uint32_t color) {
  constexpr float kReferenceWidth = 346.0f;
  constexpr float kReferenceHeight = 270.0f;
  constexpr float kStrokeWidth = 16.6f;

  const float scale = std::min(static_cast<float>(dimensions.width) / kReferenceWidth,
                               static_cast<float>(dimensions.height) / kReferenceHeight);
  const float origin_x = (static_cast<float>(dimensions.width) - kReferenceWidth * scale) * 0.5f;
  const float origin_y =
      (static_cast<float>(dimensions.height) - kReferenceHeight * scale) * 0.5f;
  auto sx = [&](float x) { return origin_x + x * scale; };
  auto sy = [&](float y) { return origin_y + y * scale; };

  canvas.setColor(0xffffffff);
  canvas.fill(0, 0, dimensions.width, dimensions.height);

  drawChevron(canvas,
              sx(127.0f),
              sy(79.0f),
              sx(168.0f),
              sy(123.0f),
              sx(209.0f),
              kStrokeWidth * scale,
              color);
  drawChevron(canvas,
              sx(127.0f),
              sy(144.0f),
              sx(168.0f),
              sy(188.0f),
              sx(209.0f),
              kStrokeWidth * scale,
              color);
}


} // namespace adt::canonical::renderers
