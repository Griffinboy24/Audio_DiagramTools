#include "audio_diagram_tools/canonical_renderers.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace adt::canonical::renderers {
using namespace drawing;
namespace {

constexpr size_t kSampleCount = 8;

constexpr float kCyclesAcrossPlot = 1.05f;
constexpr float kSineAmplitude = 0.86f;

struct SampleTablePlaybackLayout {
  Rect table_outer;
  Rect wave_outer;
  Rect wave_bevel;
  Rect wave_content;
  Rect plot;
  float cell_width = 0.0f;
  float sample_spacing = 0.0f;
  float scale = 1.0f;
};

float sampleCenterX(const SampleTablePlaybackLayout& layout, size_t index) {
  return layout.table_outer.x + layout.cell_width * (static_cast<float>(index) + 0.5f);
}

float axisY(const Rect& plot) {
  return plot.y + plot.height * 0.5f;
}

float amplitudeScale(const Rect& plot) {
  return plot.height * 0.42f;
}

float idealizedSineAmplitude(float normalized_x) {
  return kSineAmplitude * std::sin(2.0f * kPi * kCyclesAcrossPlot * normalized_x);
}

float sineSlopeYPerX(const Rect& plot, float normalized_x) {
  const float scale = amplitudeScale(plot);
  const float phase = 2.0f * kPi * kCyclesAcrossPlot * normalized_x;
  return -scale * kSineAmplitude * 2.0f * kPi * kCyclesAcrossPlot *
         std::cos(phase) / plot.width;
}

SignalPoint sinePoint(const Rect& plot, float normalized_x) {
  const float value = idealizedSineAmplitude(normalized_x);
  return { normalized_x,
           plot.x + plot.width * normalized_x,
           axisY(plot) - value * amplitudeScale(plot),
           value };
}

SignalPoint pointForSample(const SampleTablePlaybackLayout& layout, size_t index) {
  const float t = static_cast<float>(index) / static_cast<float>(kSampleCount - 1);
  return sinePoint(layout.plot, t);
}

std::string sampleLabel(size_t index) {
  const float t = static_cast<float>(index) / static_cast<float>(kSampleCount - 1);
  float value = idealizedSineAmplitude(t);
  if (std::abs(value) < 0.005f)
    value = 0.0f;

  char buffer[8] {};
  std::snprintf(buffer, sizeof(buffer), "%.2f", value);
  return buffer;
}

void appendSineBezier(visage::Path& path, const Rect& plot, float start_t, float end_t) {
  const SignalPoint start = sinePoint(plot, start_t);
  const SignalPoint end = sinePoint(plot, end_t);
  const float dx = end.x - start.x;
  const float start_slope = sineSlopeYPerX(plot, start_t);
  const float end_slope = sineSlopeYPerX(plot, end_t);

  path.bezierTo(start.x + dx / 3.0f,
                start.y + start_slope * dx / 3.0f,
                end.x - dx / 3.0f,
                end.y - end_slope * dx / 3.0f,
                end.x,
                end.y);
}

void appendSineRange(visage::Path& path,
                     const Rect& plot,
                     float start_t,
                     float end_t,
                     int segments) {
  segments = std::max(1, segments);
  for (int i = 0; i < segments; ++i) {
    const float t0 = start_t + (end_t - start_t) * static_cast<float>(i) /
                                   static_cast<float>(segments);
    const float t1 = start_t + (end_t - start_t) * static_cast<float>(i + 1) /
                                   static_cast<float>(segments);
    appendSineBezier(path, plot, t0, t1);
  }
}

visage::Path sineWavePath(const Rect& plot, float start_t, float end_t) {
  constexpr int kSegments = 32;
  const float start = std::clamp(start_t, 0.0f, 1.0f);
  const float end = std::clamp(end_t, 0.0f, 1.0f);
  const SignalPoint start_point = sinePoint(plot, start);

  visage::Path path;
  path.moveTo(start_point.x, start_point.y);
  if (end > start)
    appendSineRange(path, plot, start, end,
                    std::max(1, static_cast<int>(std::ceil((end - start) * kSegments))));
  return path;
}

std::vector<float> sineLobeBreakpoints() {
  std::vector<float> breakpoints { 0.0f };
  for (int zero = 1;; ++zero) {
    const float t = static_cast<float>(zero) / (2.0f * kCyclesAcrossPlot);
    if (t >= 1.0f)
      break;
    breakpoints.push_back(t);
  }
  breakpoints.push_back(1.0f);
  return breakpoints;
}

void drawSineWaveFill(visage::Canvas& canvas, const Rect& plot) {
  const float center_y = axisY(plot);
  const visage::Brush positive_fill = visage::Brush::linear(0x98718fd8, 0x18718fd8,
                                                            { plot.x, plot.y },
                                                            { plot.x, center_y });
  const visage::Brush negative_fill =
      visage::Brush::linear(0x18718fd8, 0x98718fd8,
                            { plot.x, center_y },
                            { plot.x, plot.y + plot.height });

  const std::vector<float> breakpoints = sineLobeBreakpoints();
  for (size_t i = 0; i + 1 < breakpoints.size(); ++i) {
    const float start_t = breakpoints[i];
    const float end_t = breakpoints[i + 1];
    const float midpoint = (start_t + end_t) * 0.5f;
    const bool positive = idealizedSineAmplitude(midpoint) >= 0.0f;
    const SignalPoint start = sinePoint(plot, start_t);
    const SignalPoint end = sinePoint(plot, end_t);
    const int segments = std::max(2, static_cast<int>(std::ceil((end_t - start_t) * 18.0f)));

    visage::Path fill;
    fill.moveTo(start.x, center_y);
    fill.lineTo(start.x, start.y);
    appendSineRange(fill, plot, start_t, end_t, segments);
    fill.lineTo(end.x, center_y);
    fill.close();

    canvas.setColor(positive ? positive_fill : negative_fill);
    canvas.fill(fill);
  }
}

void drawSineWaveformTrace(DrawContext& context, const visage::Path& path, float scale) {
  visage::Region& wide_bloom = addBlurRegion(context, 5.5f * scale);
  drawInRegion(context, wide_bloom, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, path, 5.0f * scale, 0x58718fd8);
  });

  visage::Region& edge_bloom = addBlurRegion(context, 2.1f * scale);
  drawInRegion(context, edge_bloom, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, path, 3.1f * scale, 0x789bbcff);
  });

  visage::Region& foreground = addRegion(context, true);
  drawInRegion(context, foreground, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, path, 2.0f * scale, 0xffb3c5f4);
    fillStroke(region_canvas, path, 0.55f * scale, 0xe8e4ecfb);
  });
}

void drawSineSamplePoints(DrawContext& context, const SampleTablePlaybackLayout& layout) {
  constexpr float kDotRadius = 6.5f;
  visage::Region& points_region = addRegion(context, true);
  drawInRegion(context, points_region, [&](visage::Canvas& region_canvas) {
    region_canvas.setColor(0xfff4f8ff);
    for (size_t i = 0; i < kSampleCount; ++i) {
      const SignalPoint point = pointForSample(layout, i);
      const float radius = kDotRadius * layout.scale;
      region_canvas.circle(point.x - radius, point.y - radius, radius * 2.0f);
    }
  });
}

int activeSampleIndex(const Timeline& timeline) {
  const float phase = static_cast<float>(timeline.normalized_time) -
                      std::floor(static_cast<float>(timeline.normalized_time));
  return std::min(static_cast<int>(kSampleCount - 1),
                  static_cast<int>(phase * static_cast<float>(kSampleCount)));
}

void drawAlignedTable(DrawContext& context,
                      const SampleTablePlaybackLayout& layout,
                      int active_index) {
  const float border_width = 4.0f * layout.scale;
  const float radius = 24.0f * layout.scale;

  visage::Region& shadow = addBlurRegion(context, 8.0f * layout.scale);
  drawInRegion(context, shadow, [&](visage::Canvas& shadow_canvas) {
    fillRoundedRectPath(shadow_canvas,
                        layout.table_outer.x,
                        layout.table_outer.y + 7.0f * layout.scale,
                        layout.table_outer.width,
                        layout.table_outer.height,
                        radius,
                        0x17000000);
  });

  visage::Region& table = addRegion(context, true);
  drawInRegion(context, table, [&](visage::Canvas& table_canvas) {
    fillRoundedRectPath(table_canvas,
                        layout.table_outer.x,
                        layout.table_outer.y,
                        layout.table_outer.width,
                        layout.table_outer.height,
                        radius,
                        visage::Brush::vertical(0xff838383, 0xff646567));
    fillRoundedRectPath(table_canvas,
                        layout.table_outer.x + border_width,
                        layout.table_outer.y + border_width,
                        layout.table_outer.width - border_width * 2.0f,
                        layout.table_outer.height - border_width * 2.0f,
                        radius - border_width,
                        visage::Brush::vertical(0xfffcfcfc, 0xfff4f4f7));

    const float active_x =
        layout.table_outer.x + layout.cell_width * static_cast<float>(active_index);
    const bool first_cell = active_index == 0;
    const bool last_cell = active_index == static_cast<int>(kSampleCount - 1);
    const float active_left_inset = first_cell ? border_width : 0.8f * layout.scale;
    const float active_right_inset = last_cell ? border_width : 0.8f * layout.scale;
    const Rect active_cell { active_x + active_left_inset,
                             layout.table_outer.y + border_width,
                             layout.cell_width - active_left_inset - active_right_inset,
                             layout.table_outer.height - border_width * 2.0f };
    table_canvas.setColor(0x365f83dc);
    table_canvas.fill(active_cell.x, active_cell.y, active_cell.width, active_cell.height);

    table_canvas.setColor(0xff656668);
    const float separator_top = layout.table_outer.y + border_width * 0.55f;
    const float separator_height = layout.table_outer.height - border_width * 1.10f;
    for (size_t i = 1; i < kSampleCount; ++i) {
      const float x = layout.table_outer.x + layout.cell_width * static_cast<float>(i);
      table_canvas.fill(x - 2.2f * layout.scale,
                        separator_top,
                        4.4f * layout.scale,
                        separator_height);
    }

    for (size_t i = 0; i < kSampleCount; ++i) {
      const bool active = static_cast<int>(i) == active_index;
      const float center_x = sampleCenterX(layout, i);
      text(table_canvas,
           sampleLabel(i),
           24.0f * layout.scale,
           active ? 0xff223465 : 0xff2f2f31,
           visage::Font::kCenter,
           center_x - layout.cell_width * 0.46f,
           layout.table_outer.y + 1.0f * layout.scale,
           layout.cell_width * 0.92f,
           layout.table_outer.height);
    }
  });
}

void drawAlignedWaveform(DrawContext& context,
                         const SampleTablePlaybackLayout& layout,
                         int active_index) {
  visage::Canvas& canvas = context.canvas;
  const float radius = layout.wave_outer.height * 0.052f;

  visage::Region& shadow = addBlurRegion(context, 8.0f * layout.scale);
  drawInRegion(context, shadow, [&](visage::Canvas& shadow_canvas) {
    fillRoundedRectPath(shadow_canvas,
                        layout.wave_outer.x + 7.0f * layout.scale,
                        layout.wave_outer.y + 10.0f * layout.scale,
                        layout.wave_outer.width - 14.0f * layout.scale,
                        layout.wave_outer.height,
                        radius,
                        0x25040a10);
  });

  DiagramFrameLayout frame;
  frame.outer = layout.wave_outer;
  frame.bevel = layout.wave_bevel;
  frame.content = layout.wave_content;
  frame.plot = layout.plot;
  frame.radius = radius;
  drawDiagramFrame(canvas, frame);

  const float lane_left =
      std::max(layout.plot.x,
               sampleCenterX(layout, static_cast<size_t>(active_index)) -
                   layout.sample_spacing * 0.5f);
  const float lane_right =
      std::min(layout.plot.x + layout.plot.width,
               sampleCenterX(layout, static_cast<size_t>(active_index)) +
                   layout.sample_spacing * 0.5f);
  canvas.setColor(0x345f83dc);
  canvas.fill(lane_left, layout.plot.y, lane_right - lane_left, layout.plot.height);

  drawTimelineGrid(canvas, layout.plot, kSampleCount);
  drawTimelineZeroAxis(canvas, layout.plot);

  drawSineWaveFill(canvas, layout.plot);
  const visage::Path waveform = sineWavePath(layout.plot, 0.0f, 1.0f);
  drawSineWaveformTrace(context, waveform, layout.scale);

  const float active_start =
      std::max(0.0f, (static_cast<float>(active_index) - 0.5f) /
                         static_cast<float>(kSampleCount - 1));
  const float active_end =
      std::min(1.0f, (static_cast<float>(active_index) + 0.5f) /
                         static_cast<float>(kSampleCount - 1));
  const visage::Path active_path = sineWavePath(layout.plot, active_start, active_end);
  fillStroke(canvas, active_path, 5.0f * layout.scale, 0x7086a8ff);
  fillStroke(canvas, active_path, 2.1f * layout.scale, 0xffbfd0ff);
  fillStroke(canvas, active_path, 0.65f * layout.scale, 0xffffffff);

  drawSineSamplePoints(context, layout);

  drawDiagramFrameCorners(canvas, frame);
}

SampleTablePlaybackLayout makeLayout(const Dimensions& dimensions) {
  constexpr float kReferenceWidth = 920.0f;
  constexpr float kReferenceHeight = 520.0f;
  const float scale = std::min(static_cast<float>(dimensions.width) / kReferenceWidth,
                               static_cast<float>(dimensions.height) / kReferenceHeight);
  const float origin_x =
      (static_cast<float>(dimensions.width) - kReferenceWidth * scale) * 0.5f;
  const float origin_y =
      (static_cast<float>(dimensions.height) - kReferenceHeight * scale) * 0.5f;
  auto sx = [&](float x) { return origin_x + x * scale; };
  auto sy = [&](float y) { return origin_y + y * scale; };

  const Rect wave_outer { sx(78.0f), sy(218.0f), 764.0f * scale, 236.0f * scale };
  const Rect wave_bevel = insetRect(wave_outer, 3.0f * scale, 3.8f * scale);
  const Rect wave_content = insetRect(wave_bevel, 3.0f * scale, 3.6f * scale);
  const float plot_x = std::max(18.0f * scale, wave_outer.width * 0.027f);
  const float plot_top = std::max(13.0f * scale, wave_outer.height * 0.064f);
  const float plot_bottom = std::max(10.0f * scale, wave_outer.height * 0.050f);
  const Rect plot { wave_content.x + plot_x,
                    wave_content.y + plot_top,
                    wave_content.width - 2.0f * plot_x,
                    wave_content.height - plot_top - plot_bottom };
  const float sample_spacing = plot.width / static_cast<float>(kSampleCount - 1);
  const float cell_width = sample_spacing;
  const Rect table { plot.x - cell_width * 0.5f,
                     sy(62.0f),
                     cell_width * static_cast<float>(kSampleCount),
                     72.0f * scale };
  return { table, wave_outer, wave_bevel, wave_content, plot, cell_width,
           sample_spacing, scale };
}

} // namespace

void drawSampleTablePlaybackScene(DrawContext& context,
                                  const Dimensions& dimensions,
                                  const Timeline& timeline) {
  visage::Canvas& canvas = context.canvas;
  canvas.setColor(0xffffffff);
  canvas.fill(0, 0, dimensions.width, dimensions.height);

  const SampleTablePlaybackLayout layout = makeLayout(dimensions);
  const int active_index = activeSampleIndex(timeline);
  drawAlignedTable(context, layout, active_index);
  drawChevron(canvas,
              layout.table_outer.x + layout.table_outer.width * 0.5f - 12.0f * layout.scale,
              layout.table_outer.y + layout.table_outer.height + 36.0f * layout.scale,
              layout.table_outer.x + layout.table_outer.width * 0.5f,
              layout.table_outer.y + layout.table_outer.height + 48.0f * layout.scale,
              layout.table_outer.x + layout.table_outer.width * 0.5f + 12.0f * layout.scale,
              4.4f * layout.scale,
              0xff4e4e4e);
  drawAlignedWaveform(context, layout, active_index);
}

} // namespace adt::canonical::renderers
