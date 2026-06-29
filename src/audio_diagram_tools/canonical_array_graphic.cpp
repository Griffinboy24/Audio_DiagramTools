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

void drawArrayGraphic(DrawContext& context, const Dimensions& dimensions) {
  constexpr std::array<std::string_view, 8> kValues {
    "0.00", "0.42", "0.88", "0.56", "0.12", "0.76", "0.48", "0.22"
  };
  constexpr float kReferenceWidth = 1668.0f;
  constexpr float kReferenceHeight = 388.0f;
  constexpr Rect kReferenceOuter { 110.0f, 120.0f, 1405.0f, 140.0f };
  constexpr std::array<float, 7> kReferenceSeparators {
    289.5f, 463.5f, 637.0f, 812.5f, 986.5f, 1161.0f, 1335.0f
  };
  constexpr std::array<float, 8> kReferenceValueCenters {
    203.0f, 380.5f, 554.5f, 725.0f, 898.5f, 1077.0f, 1249.5f, 1426.5f
  };
  constexpr float kBorderWidth = 6.0f;
  constexpr float kSeparatorWidth = 5.5f;
  constexpr float kRadius = 49.0f;
  constexpr float kTextSize = 49.5f;
  constexpr float kTextBoxWidth = 150.0f;

  const float scale = std::min(static_cast<float>(dimensions.width) / kReferenceWidth,
                               static_cast<float>(dimensions.height) / kReferenceHeight);
  const float origin_x = (static_cast<float>(dimensions.width) - kReferenceWidth * scale) * 0.5f;
  const float origin_y = (static_cast<float>(dimensions.height) - kReferenceHeight * scale) * 0.5f;
  const Rect outer { origin_x + kReferenceOuter.x * scale,
                     origin_y + kReferenceOuter.y * scale,
                     kReferenceOuter.width * scale,
                     kReferenceOuter.height * scale };
  const float border_width = kBorderWidth * scale;
  const float radius = kRadius * scale;
  const float separator_width = kSeparatorWidth * scale;

  visage::Region& shadow = addBlurRegion(context, 10.0f * scale);
  drawInRegion(context, shadow, [&](visage::Canvas& canvas) {
    fillRoundedRectPath(canvas,
                        outer.x,
                        outer.y + 9.6f * scale,
                        outer.width,
                        outer.height,
                        radius,
                        0x18000000);
  });

  visage::Region& foreground = addRegion(context, true);
  drawInRegion(context, foreground, [&](visage::Canvas& canvas) {
  fillRoundedRectPath(canvas,
                      outer.x,
                      outer.y,
                      outer.width,
                      outer.height,
                      radius,
                      visage::Brush::vertical(0xff838383, 0xff646567));

  fillRoundedRectPath(canvas,
                      outer.x + border_width,
                      outer.y + border_width,
                      outer.width - border_width * 2.0f,
                      outer.height - border_width * 2.0f,
                      radius - border_width,
                      visage::Brush::vertical(0xfffcfcfc, 0xfff4f4f7));

  canvas.setColor(0xff656668);
  const float separator_top = outer.y + border_width * 0.52f;
  const float separator_height = outer.height - border_width * 1.04f;
  for (float separator_x : kReferenceSeparators) {
    const float x = origin_x + separator_x * scale;
    canvas.fill(x - separator_width * 0.5f,
                separator_top,
                separator_width,
                separator_height);
  }

  const float text_y = outer.y + 1.3f * scale;
  const float text_box_width = kTextBoxWidth * scale;
  for (size_t i = 0; i < kValues.size(); ++i) {
    const float center_x = origin_x + kReferenceValueCenters[i] * scale;
    text(canvas,
         std::string(kValues[i]),
         kTextSize * scale,
         0xff2f2f31,
         visage::Font::kCenter,
         center_x - text_box_width * 0.5f,
         text_y,
         text_box_width,
         outer.height);
  }
  });
}


} // namespace adt::canonical::renderers
