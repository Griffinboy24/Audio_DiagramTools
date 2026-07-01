#include "audio_diagram_tools/canonical_renderers.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace adt::canonical::renderers {
using namespace drawing;
namespace {

constexpr std::size_t kSampleCount = 54;
constexpr std::size_t kTableColumns = 66;

struct DenseSampleLayout {
  Rect table;
  Rect wave_outer;
  Rect wave_bevel;
  Rect wave_content;
  Rect plot;
  float table_cell = 0.0f;
  float sample_spacing = 0.0f;
  float scale = 1.0f;
};

float axisY(const Rect& plot) {
  return plot.y + plot.height * 0.5f;
}

float amplitudeScale(const Rect& plot) {
  return plot.height * 0.46f;
}

float denseSineValue(float t) {
  const float phase = 2.0f * kPi * (0.98f * t + 0.018f);
  const float slow_shape = 0.045f * std::sin(2.0f * kPi * (0.22f * t + 0.13f));
  return std::clamp(0.88f * std::sin(phase) + slow_shape, -0.94f, 0.94f);
}

std::string sampleLabel(float value) {
  if (std::abs(value) < 0.005f)
    value = 0.0f;

  char buffer[16] {};
  std::snprintf(buffer, sizeof(buffer), "%.1f", value);
  std::string label(buffer);
  if (label == "0.0" || label == "-0.0")
    return "0";
  return label;
}

float sampleX(const DenseSampleLayout& layout, std::size_t index) {
  return layout.plot.x + layout.sample_spacing * static_cast<float>(index);
}

SignalPoint samplePoint(const DenseSampleLayout& layout, std::size_t index) {
  const float t = static_cast<float>(index) / static_cast<float>(kSampleCount - 1);
  const float value = denseSineValue(t);
  return { t, sampleX(layout, index), axisY(layout.plot) - value * amplitudeScale(layout.plot),
           value };
}

std::vector<SignalPoint> makeWaveform(const DenseSampleLayout& layout) {
  constexpr int kPointCount = 860;
  std::vector<SignalPoint> points;
  points.reserve(kPointCount);
  for (int i = 0; i < kPointCount; ++i) {
    const float t = static_cast<float>(i) / static_cast<float>(kPointCount - 1);
    const float value = denseSineValue(t);
    points.push_back({ t,
                       layout.plot.x + layout.plot.width * t,
                       axisY(layout.plot) - value * amplitudeScale(layout.plot),
                       value });
  }
  return points;
}

void drawDenseSampleTable(DrawContext& context, const DenseSampleLayout& layout) {
  visage::Canvas& canvas = context.canvas;
  const float border_width = 2.6f * layout.scale;
  const float separator_width = 1.55f * layout.scale;
  const float radius = 12.0f * layout.scale;

  visage::Region& shadow = addBlurRegion(context, 5.0f * layout.scale);
  drawInRegion(context, shadow, [&](visage::Canvas& shadow_canvas) {
    fillRoundedRectPath(shadow_canvas,
                        layout.table.x,
                        layout.table.y + 5.0f * layout.scale,
                        layout.table.width,
                        layout.table.height,
                        radius,
                        0x12000000);
  });

  fillRoundedRectPath(canvas,
                      layout.table.x,
                      layout.table.y,
                      layout.table.width,
                      layout.table.height,
                      radius,
                      visage::Brush::vertical(0xff838383, 0xff646567));
  fillRoundedRectPath(canvas,
                      layout.table.x + border_width,
                      layout.table.y + border_width,
                      layout.table.width - border_width * 2.0f,
                      layout.table.height - border_width * 2.0f,
                      radius - border_width,
                      visage::Brush::vertical(0xfffcfcfc, 0xfff4f4f7));

  canvas.setColor(0xff68696b);
  const float separator_top = layout.table.y + border_width * 0.60f;
  const float separator_height = layout.table.height - border_width * 1.20f;
  for (std::size_t i = 1; i < kTableColumns; ++i) {
    const float x = layout.table.x + layout.table_cell * static_cast<float>(i);
    canvas.fill(x - separator_width * 0.5f,
                separator_top,
                separator_width,
                separator_height);
  }

  for (std::size_t i = 0; i < kTableColumns; ++i) {
    const float value =
        denseSineValue(static_cast<float>(i) / static_cast<float>(kTableColumns - 1));
    const float cell_x = layout.table.x + layout.table_cell * static_cast<float>(i);
    text(canvas,
         sampleLabel(value),
         8.6f * layout.scale,
         0xff343437,
         visage::Font::kCenter,
         cell_x + 1.0f * layout.scale,
         layout.table.y,
         layout.table_cell - 2.0f * layout.scale,
         layout.table_cell);
  }
}

void drawDenseWaveformPanel(DrawContext& context, const DenseSampleLayout& layout) {
  visage::Canvas& canvas = context.canvas;
  DiagramFrameLayout frame;
  frame.outer = layout.wave_outer;
  frame.bevel = layout.wave_bevel;
  frame.content = layout.wave_content;
  frame.plot = layout.plot;
  frame.radius = 8.0f * layout.scale;

  visage::Region& shadow = addBlurRegion(context, 6.5f * layout.scale);
  drawInRegion(context, shadow, [&](visage::Canvas& shadow_canvas) {
    fillRoundedRectPath(shadow_canvas,
                        layout.wave_outer.x + 6.0f * layout.scale,
                        layout.wave_outer.y + 8.0f * layout.scale,
                        layout.wave_outer.width - 12.0f * layout.scale,
                        layout.wave_outer.height,
                        frame.radius,
                        0x22040a10);
  });

  drawDiagramFrame(canvas, frame);
  drawTimelineGrid(canvas, layout.plot, kSampleCount);
  drawTimelineZeroAxis(canvas, layout.plot);

  drawGriffinWaveformTrace(context, makeWaveform(layout), layout.plot, layout.scale);

  visage::Region& dots = addRegion(context, true);
  drawInRegion(context, dots, [&](visage::Canvas& dot_canvas) {
    dot_canvas.setColor(0xfff7faff);
    for (std::size_t i = 0; i < kSampleCount; ++i) {
      const SignalPoint point = samplePoint(layout, i);
      const float radius = 3.2f * layout.scale;
      dot_canvas.circle(point.x - radius, point.y - radius, radius * 2.0f);
    }
  });

  drawDiagramFrameCorners(canvas, frame);
}

void drawRightFade(DrawContext& context, const DenseSampleLayout& layout) {
  const float fade_start = static_cast<float>(context.dimensions.width) - 88.0f * layout.scale;
  const float fade_width = static_cast<float>(context.dimensions.width) - fade_start;

  visage::Region& fade = addRegion(context, true);
  drawInRegion(context, fade, [&](visage::Canvas& canvas) {
    canvas.setColor(visage::Brush::linear(0x00ffffff,
                                          0xffffffff,
                                          { fade_start, 0.0f },
                                          { static_cast<float>(context.dimensions.width) -
                                                12.0f * layout.scale,
                                            0.0f }));
    canvas.fill(fade_start, 0.0f, fade_width, static_cast<float>(context.dimensions.height));
    canvas.setColor(0xffffffff);
    canvas.fill(static_cast<float>(context.dimensions.width) - 12.0f * layout.scale,
                0.0f,
                12.0f * layout.scale,
                static_cast<float>(context.dimensions.height));
  });
}

void drawScaleLabel(visage::Canvas& canvas, const DenseSampleLayout& layout) {
  const Rect label_area { 0.0f,
                          268.0f * layout.scale,
                          920.0f * layout.scale,
                          74.0f * layout.scale };
  fauxBoldText(canvas,
               "\"imagine this goes on for 48k samples...",
               25.0f * layout.scale,
               0xff171b24,
               visage::Font::kCenter,
               label_area.x,
               label_area.y,
               label_area.width,
               34.0f * layout.scale);
  text(canvas,
       "That's one second of audio\"",
       25.0f * layout.scale,
       0xff171b24,
       visage::Font::kCenter,
       label_area.x,
       label_area.y + 36.0f * layout.scale,
       label_area.width,
       34.0f * layout.scale);
}

DenseSampleLayout makeLayout(const Dimensions& dimensions) {
  constexpr float kReferenceWidth = 920.0f;
  constexpr float kReferenceHeight = 380.0f;
  const float scale = std::min(static_cast<float>(dimensions.width) / kReferenceWidth,
                               static_cast<float>(dimensions.height) / kReferenceHeight);
  const float origin_x =
      (static_cast<float>(dimensions.width) - kReferenceWidth * scale) * 0.5f;
  const float origin_y =
      (static_cast<float>(dimensions.height) - kReferenceHeight * scale) * 0.5f;
  auto sx = [&](float x) { return origin_x + x * scale; };
  auto sy = [&](float y) { return origin_y + y * scale; };

  const Rect wave_outer { sx(58.0f), sy(108.0f), 1090.0f * scale, 118.0f * scale };
  const Rect wave_bevel = insetRect(wave_outer, 3.0f * scale, 3.8f * scale);
  const Rect wave_content = insetRect(wave_bevel, 3.0f * scale, 3.6f * scale);
  const float plot_x = std::max(18.0f * scale, wave_outer.width * 0.026f);
  const float plot_top = std::max(13.0f * scale, wave_outer.height * 0.064f);
  const float plot_bottom = std::max(10.0f * scale, wave_outer.height * 0.050f);
  const Rect plot { wave_content.x + plot_x,
                    wave_content.y + plot_top,
                    wave_content.width - 2.0f * plot_x,
                    wave_content.height - plot_top - plot_bottom };
  const float sample_spacing = plot.width / static_cast<float>(kSampleCount - 1);
  const float table_cell = 24.0f * scale;
  const Rect table { sx(58.0f),
                     sy(44.0f),
                     table_cell * static_cast<float>(kTableColumns),
                     table_cell };

  return { table, wave_outer, wave_bevel, wave_content, plot, table_cell, sample_spacing,
           scale };
}

} // namespace

void drawDenseSampleWaveformGraphic(DrawContext& context, const Dimensions& dimensions) {
  visage::Canvas& canvas = context.canvas;
  canvas.setColor(0xffffffff);
  canvas.fill(0, 0, dimensions.width, dimensions.height);

  const DenseSampleLayout layout = makeLayout(dimensions);
  drawDenseSampleTable(context, layout);
  drawDenseWaveformPanel(context, layout);
  drawRightFade(context, layout);
  drawScaleLabel(canvas, layout);
}

} // namespace adt::canonical::renderers
