#include "audio_diagram_tools/canonical_renderers.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace adt::canonical::renderers {
using namespace drawing;
namespace {

constexpr std::size_t kValueCount = kSampleTableWaveformValueCount;
constexpr std::size_t kTableColumns = kValueCount;
constexpr std::size_t kTableRows = 1;

struct SampleTableWaveformLayout {
  Rect table;
  Rect wave_outer;
  Rect wave_bevel;
  Rect wave_content;
  Rect plot;
  float cell_width = 0.0f;
  float sample_spacing = 0.0f;
  float scale = 1.0f;
};

float axisY(const Rect& plot) {
  return plot.y + plot.height * 0.5f;
}

float amplitudeScale(const Rect& plot) {
  return plot.height * 0.42f;
}

float sampleX(const SampleTableWaveformLayout& layout, std::size_t index) {
  const float t = static_cast<float>(index) / static_cast<float>(kValueCount - 1);
  return layout.plot.x + layout.plot.width * t;
}

float sampleSeparatorX(const SampleTableWaveformLayout& layout, std::size_t separator_index) {
  return (sampleX(layout, separator_index - 1) + sampleX(layout, separator_index)) * 0.5f;
}

float normalizedValue(const SampleTableWaveformOptions& options, float value) {
  const float max_abs = std::max(0.001f, options.max_abs_value);
  return std::clamp(value / max_abs, -1.0f, 1.0f);
}

std::string sampleLabel(float value) {
  if (std::abs(value) < 0.005f)
    value = 0.0f;

  char buffer[16] {};
  std::snprintf(buffer, sizeof(buffer), "%.2f", value);
  std::string label(buffer);
  while (label.size() > 1 && label.back() == '0')
    label.pop_back();
  if (!label.empty() && label.back() == '.')
    label.pop_back();
  return label;
}

float interpolatedSampleValue(const SampleTableWaveformOptions& options, float t) {
  const float position = std::clamp(t, 0.0f, 1.0f) * static_cast<float>(kValueCount - 1);
  const int index = static_cast<int>(std::floor(position));
  const float amount = position - static_cast<float>(index);
  auto sample = [&](int sample_index) {
    const int clamped_index =
        std::clamp(sample_index, 0, static_cast<int>(kValueCount - 1));
    return normalizedValue(options, options.values[static_cast<std::size_t>(clamped_index)]);
  };

  const float p0 = sample(index - 1);
  const float p1 = sample(index);
  const float p2 = sample(index + 1);
  const float p3 = sample(index + 2);
  const float t2 = amount * amount;
  const float t3 = t2 * amount;
  const float value = 0.5f * ((2.0f * p1) +
                              (-p0 + p2) * amount +
                              (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
                              (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
  return std::clamp(value, -1.0f, 1.0f);
}

SignalPoint pointAt(const SampleTableWaveformLayout& layout,
                    const SampleTableWaveformOptions& options,
                    float t) {
  const float value = interpolatedSampleValue(options, t);
  return { t,
           layout.plot.x + layout.plot.width * t,
           axisY(layout.plot) - value * amplitudeScale(layout.plot),
           value };
}

SignalPoint pointForSample(const SampleTableWaveformLayout& layout,
                           const SampleTableWaveformOptions& options,
                           std::size_t index) {
  const float t = static_cast<float>(index) / static_cast<float>(kValueCount - 1);
  const float value = normalizedValue(options, options.values[index]);
  return { t,
           layout.plot.x + layout.plot.width * t,
           axisY(layout.plot) - value * amplitudeScale(layout.plot),
           value };
}

std::vector<SignalPoint> makeWaveform(const SampleTableWaveformLayout& layout,
                                      const SampleTableWaveformOptions& options) {
  constexpr int kSamples = 520;
  std::vector<SignalPoint> points;
  points.reserve(kSamples);
  for (int i = 0; i < kSamples; ++i) {
    const float t = static_cast<float>(i) / static_cast<float>(kSamples - 1);
    points.push_back(pointAt(layout, options, t));
  }
  return points;
}

void drawSampleTable(visage::Canvas& canvas,
                     const SampleTableWaveformLayout& layout,
                     const SampleTableWaveformOptions& options) {
  const float border_width = 3.8f * layout.scale;
  const float separator_width = 3.1f * layout.scale;
  const float radius = 20.0f * layout.scale;
  const float row_height = layout.table.height / static_cast<float>(kTableRows);

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

  canvas.setColor(0xff656668);
  const float separator_top = layout.table.y + border_width * 0.56f;
  const float separator_height = layout.table.height - border_width * 1.12f;
  for (std::size_t i = 1; i < kTableColumns; ++i) {
    const float x = sampleSeparatorX(layout, i);
    canvas.fill(x - separator_width * 0.5f,
                separator_top,
                separator_width,
                separator_height);
  }

  for (std::size_t i = 0; i < kValueCount; ++i) {
    const float cell_left = i == 0 ? layout.table.x + border_width * 0.75f
                                   : sampleSeparatorX(layout, i);
    const float cell_right = i + 1 == kValueCount
                                 ? layout.table.x + layout.table.width - border_width * 0.75f
                                 : sampleSeparatorX(layout, i + 1);
    const float cell_y = layout.table.y;
    text(canvas,
         sampleLabel(options.values[i]),
         21.0f * layout.scale,
         0xff2f2f31,
         visage::Font::kCenter,
         cell_left,
         cell_y + 1.0f * layout.scale,
         cell_right - cell_left,
         row_height - 1.0f * layout.scale);
  }
}

void drawSampleDots(DrawContext& context,
                    const SampleTableWaveformLayout& layout,
                    const SampleTableWaveformOptions& options) {
  visage::Region& dot_region = addRegion(context, true);
  drawInRegion(context, dot_region, [&](visage::Canvas& canvas) {
    canvas.setColor(0xfff4f8ff);
    for (std::size_t i = 0; i < kValueCount; ++i) {
      const SignalPoint point = pointForSample(layout, options, i);
      const float radius = 5.9f * layout.scale;
      canvas.circle(point.x - radius, point.y - radius, radius * 2.0f);
    }
  });
}

void drawWaveformPanel(DrawContext& context,
                       const SampleTableWaveformLayout& layout,
                       const SampleTableWaveformOptions& options) {
  visage::Canvas& canvas = context.canvas;
  DiagramFrameLayout frame;
  frame.outer = layout.wave_outer;
  frame.bevel = layout.wave_bevel;
  frame.content = layout.wave_content;
  frame.plot = layout.plot;
  frame.radius = 8.4f * layout.scale;

  visage::Region& shadow = addBlurRegion(context, 6.5f * layout.scale);
  drawInRegion(context, shadow, [&](visage::Canvas& shadow_canvas) {
    fillRoundedRectPath(shadow_canvas,
                        layout.wave_outer.x + 5.0f * layout.scale,
                        layout.wave_outer.y + 8.0f * layout.scale,
                        layout.wave_outer.width - 10.0f * layout.scale,
                        layout.wave_outer.height,
                        frame.radius,
                        0x24040a10);
  });

  drawDiagramFrame(canvas, frame);
  drawTimelineGrid(canvas, layout.plot, kValueCount);
  drawTimelineZeroAxis(canvas, layout.plot);

  const std::vector<SignalPoint> waveform = makeWaveform(layout, options);
  drawGriffinWaveformTrace(context, waveform, layout.plot, layout.scale);
  drawSampleDots(context, layout, options);
  drawDiagramFrameCorners(canvas, frame);
}

SampleTableWaveformLayout makeLayout(const Dimensions& dimensions) {
  constexpr float kReferenceWidth = 820.0f;
  constexpr float kReferenceHeight = 330.0f;
  const float scale = std::min(static_cast<float>(dimensions.width) / kReferenceWidth,
                               static_cast<float>(dimensions.height) / kReferenceHeight);
  const float origin_x =
      (static_cast<float>(dimensions.width) - kReferenceWidth * scale) * 0.5f;
  const float origin_y =
      (static_cast<float>(dimensions.height) - kReferenceHeight * scale) * 0.5f;
  auto sx = [&](float x) { return origin_x + x * scale; };
  auto sy = [&](float y) { return origin_y + y * scale; };

  const Rect wave_outer { sx(28.0f), sy(102.0f), 764.0f * scale, 214.0f * scale };
  const Rect wave_bevel = insetRect(wave_outer, 3.0f * scale, 3.8f * scale);
  const Rect wave_content = insetRect(wave_bevel, 3.0f * scale, 3.6f * scale);
  const float plot_x = std::max(18.0f * scale, wave_outer.width * 0.027f);
  const float plot_top = std::max(13.0f * scale, wave_outer.height * 0.064f);
  const float plot_bottom = std::max(10.0f * scale, wave_outer.height * 0.050f);
  const Rect plot { wave_content.x + plot_x,
                    wave_content.y + plot_top,
                    wave_content.width - 2.0f * plot_x,
                    wave_content.height - plot_top - plot_bottom };
  const float sample_spacing = plot.width / static_cast<float>(kValueCount - 1);
  const float cell_width = sample_spacing;
  const float natural_table_x = plot.x - cell_width * 0.5f;
  const float natural_table_right =
      natural_table_x + cell_width * static_cast<float>(kValueCount);
  constexpr float kEdgeClampBlend = 0.50f;
  const float table_x = natural_table_x + (wave_outer.x - natural_table_x) * kEdgeClampBlend;
  const float table_right =
      natural_table_right +
      (wave_outer.x + wave_outer.width - natural_table_right) * kEdgeClampBlend;
  const Rect table { table_x, sy(12.0f), table_right - table_x, 64.0f * scale };

  return { table, wave_outer, wave_bevel, wave_content, plot, cell_width,
           sample_spacing, scale };
}

} // namespace

void drawSampleTableWaveformGraphic(DrawContext& context,
                                    const Dimensions& dimensions,
                                    const SampleTableWaveformOptions& options) {
  visage::Canvas& canvas = context.canvas;
  if (options.clear_background) {
    canvas.setColor(0xffffffff);
    canvas.fill(0, 0, dimensions.width, dimensions.height);
  }

  const SampleTableWaveformLayout layout = makeLayout(dimensions);
  drawSampleTable(canvas, layout, options);
  drawWaveformPanel(context, layout, options);
}

} // namespace adt::canonical::renderers
