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

void drawSampleValuesPlot(DrawContext& context, const Dimensions& dimensions) {
  struct AxisLabel {
    std::string_view label;
    float y;
  };

  struct SampleLabel {
    std::string_view label;
    float x;
    float y;
    float label_y_offset;
  };

  constexpr float kReferenceWidth = 1644.0f;
  constexpr float kReferenceHeight = 612.0f;
  constexpr Rect kReferenceCard { 130.0f, 83.0f, 1384.0f, 377.0f };
  constexpr float kAxisX = 244.0f;
  constexpr float kGuideRight = 1439.0f;
  constexpr float kZeroY = 270.0f;
  constexpr float kAxisLabelZeroInset = 0.06f;
  constexpr std::array<AxisLabel, 5> kAxisLabels {
    AxisLabel { "+1.0", 143.0f },
    AxisLabel { "+0.5", 210.0f },
    AxisLabel { "0.0", 270.0f },
    AxisLabel { "-0.5", 330.0f },
    AxisLabel { "-1.0", 397.0f },
  };
  constexpr std::array<SampleLabel, 8> kSamples {
    SampleLabel { "0.00", 305.2f, 271.0f, -53.0f },
    SampleLabel { "0.70", 456.8f, 181.0f, -53.0f },
    SampleLabel { "0.82", 605.4f, 166.0f, -56.0f },
    SampleLabel { "0.27", 757.0f, 236.0f, -54.0f },
    SampleLabel { "-0.51", 906.6f, 335.0f, 25.0f },
    SampleLabel { "-0.86", 1056.2f, 379.0f, 26.0f },
    SampleLabel { "-0.51", 1203.9f, 335.0f, 25.0f },
    SampleLabel { "0.27", 1355.4f, 236.0f, -54.0f },
  };
  const float scale = std::min(static_cast<float>(dimensions.width) / kReferenceWidth,
                               static_cast<float>(dimensions.height) / kReferenceHeight);
  const float origin_x = (static_cast<float>(dimensions.width) - kReferenceWidth * scale) * 0.5f;
  const float origin_y =
      (static_cast<float>(dimensions.height) - kReferenceHeight * scale) * 0.5f;
  auto sx = [&](float x) { return origin_x + x * scale; };
  auto sy = [&](float y) { return origin_y + y * scale; };

  visage::Canvas& canvas = context.canvas;
  canvas.setColor(0xffffffff);
  canvas.fill(0, 0, dimensions.width, dimensions.height);
  constexpr bool kShowCenterGuides = false;
  if (kShowCenterGuides) {
    canvas.setColor(0x55ff3048);
    canvas.fill(static_cast<float>(dimensions.width) * 0.5f - 0.5f, 0.0f, 1.0f,
                static_cast<float>(dimensions.height));
    canvas.fill(0.0f, static_cast<float>(dimensions.height) * 0.5f - 0.5f,
                static_cast<float>(dimensions.width), 1.0f);
  }

  const Rect card { sx(kReferenceCard.x),
                    sy(kReferenceCard.y),
                    kReferenceCard.width * scale,
                    kReferenceCard.height * scale };
  const float radius = 17.5f * scale;
  const float border_width = 5.0f * scale;

  visage::Region& shadow = addBlurRegion(context, 19.0f * scale);
  drawInRegion(context, shadow, [&](visage::Canvas& shadow_canvas) {
    fillRoundedRectPath(shadow_canvas,
                        card.x + 8.0f * scale,
                        card.y + 19.0f * scale,
                        card.width - 16.0f * scale,
                        card.height,
                        radius,
                        0x20101b2d);
  });

  visage::Region& foreground = addRegion(context, true);
  drawInRegion(context, foreground, [&](visage::Canvas& card_canvas) {
    fillRoundedRectPath(
        card_canvas, card.x, card.y, card.width, card.height, radius, 0xffeaebf0);
    fillRoundedRectPath(card_canvas,
                        card.x + border_width,
                        card.y + border_width,
                        card.width - border_width * 2.0f,
                        card.height - border_width * 2.0f,
                        radius - border_width,
                        visage::Brush::vertical(0xfff3f3f7, 0xffeff0f7));

    const float axis_x = sx(kAxisX);
    const float guide_right = sx(kGuideRight);
    const float guide_width = guide_right - axis_x;
    const float zero_y = sy(kZeroY);
    const uint32_t guide_color = 0xffc9c9c9;
    const uint32_t axis_color = 0xffa9b3c4;

    for (const AxisLabel& label : kAxisLabels) {
      if (label.y == kZeroY)
        continue;

      drawPillDashedHorizontalRule(card_canvas,
                                   axis_x,
                                   sy(label.y),
                                   guide_width,
                                   3.0f * scale,
                                   3.4f * scale,
                                   7.6f * scale,
                                   guide_color);
    }

    drawLine(card_canvas,
             axis_x,
             sy(kAxisLabels.front().y),
             axis_x,
             sy(kAxisLabels.back().y),
             3.0f * scale,
             axis_color);
    drawLine(card_canvas, axis_x, zero_y, guide_right, zero_y, 3.0f * scale, axis_color);

    for (const AxisLabel& label : kAxisLabels) {
      const float label_y = insetToward(label.y, kZeroY, kAxisLabelZeroInset);
      text(card_canvas,
           std::string(label.label),
           23.0f * scale,
           0xff0a0a0c,
           visage::Font::kCenter,
           sx(170.0f),
           sy(label_y) - 20.0f * scale,
           64.0f * scale,
           38.0f * scale);
    }

    constexpr uint32_t marker_color = 0xff4a7cd9;
    constexpr uint32_t marker_fill = 0xffffffff;
    const float marker_radius = 12.0f * scale;
    for (const SampleLabel& sample : kSamples) {
      const float x = sx(sample.x);
      const float y = sy(sample.y);
      drawDot(card_canvas, x, y, marker_radius - 2.0f * scale, marker_fill);
      drawOpenDot(card_canvas, x, y, marker_radius, marker_color, 4.2f * scale);
    }

    for (const SampleLabel& sample : kSamples) {
      text(card_canvas,
           std::string(sample.label),
           24.5f * scale,
           0xff07070a,
           visage::Font::kCenter,
           sx(sample.x) - 55.0f * scale,
           sy(sample.y + sample.label_y_offset),
           110.0f * scale,
           34.0f * scale);
    }
  });
}


} // namespace adt::canonical::renderers
