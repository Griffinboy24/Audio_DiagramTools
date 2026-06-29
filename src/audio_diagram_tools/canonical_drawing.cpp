#include "audio_diagram_tools/audio_file_motion.h"
#include "audio_diagram_tools/canonical_drawing.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <embedded/fonts.h>

namespace adt::canonical::drawing {

DrawContext::DrawContext(visage::Canvas& render_canvas, const Dimensions& render_dimensions) :
    canvas(render_canvas), dimensions(render_dimensions) { }

float clamp01(float value) {
  return std::max(0.0f, std::min(1.0f, value));
}

float smoothstep(float edge0, float edge1, float value) {
  const float t = clamp01((value - edge0) / (edge1 - edge0));
  return t * t * (3.0f - 2.0f * t);
}

float insetToward(float value, float target, float amount) {
  return value + (target - value) * amount;
}

uint32_t alphaColor(uint8_t alpha, uint32_t rgb) {
  return (static_cast<uint32_t>(alpha) << 24) | (rgb & 0x00ffffffu);
}

Rect insetRect(const Rect& rect, float x, float y) {
  return { rect.x + x, rect.y + y, rect.width - 2.0f * x, rect.height - 2.0f * y };
}

visage::Font labelFont(float size) {
  return visage::Font(size, visage::fonts::Lato_Regular_ttf);
}

void text(visage::Canvas& canvas,
          const std::string& label,
          float size,
          uint32_t color,
          visage::Font::Justification justification,
          float x,
          float y,
          float width,
          float height) {
  canvas.setColor(color);
  canvas.text(label.c_str(), labelFont(size), justification, x, y, width, height);
}

void fauxBoldText(visage::Canvas& canvas,
                  const std::string& label,
                  float size,
                  uint32_t color,
                  visage::Font::Justification justification,
                  float x,
                  float y,
                  float width,
                  float height) {
  text(canvas, label, size, color, justification, x, y, width, height);
  text(canvas, label, size, color, justification, x + 0.45f, y, width, height);
}

void fillStroke(visage::Canvas& canvas,
                const visage::Path& path,
                float width,
                uint32_t color,
                const std::vector<float>& dash,
                float dash_offset,
                visage::Path::EndCap cap) {
  visage::Path source = path;
  canvas.setColor(color);
  canvas.fill(source.stroke(width, visage::Path::Join::Round, cap, dash, dash_offset));
}

visage::Region& addRegion(DrawContext& context, bool on_top) {
  auto region = std::make_unique<visage::Region>();
  region->setBounds(0, 0, context.dimensions.width, context.dimensions.height);
  region->setOnTop(on_top);
  context.canvas.addRegion(region.get());

  visage::Region& result = *region;
  context.regions.push_back(std::move(region));
  return result;
}

visage::Region& addBlurRegion(DrawContext& context, float blur_radius) {
  auto effect = std::make_unique<visage::BlurPostEffect>();
  effect->setBlurRadius(blur_radius);

  auto region = std::make_unique<visage::Region>();
  region->setBounds(0, 0, context.dimensions.width, context.dimensions.height);
  region->setOnTop(true);
  region->setPostEffect(effect.get());
  context.canvas.addRegion(region.get());
  region->setNeedsLayer(true);

  visage::Region& result = *region;
  context.blur_effects.push_back(std::move(effect));
  context.regions.push_back(std::move(region));
  return result;
}

void drawInRegion(DrawContext& context,
                  visage::Region& region,
                  const std::function<void(visage::Canvas&)>& draw) {
  context.canvas.beginRegion(&region);
  draw(context.canvas);
  context.canvas.endRegion();
}

void drawLine(visage::Canvas& canvas,
              float x1,
              float y1,
              float x2,
              float y2,
              float width,
              uint32_t color,
              const std::vector<float>& dash) {
  visage::Path path;
  path.moveTo(x1, y1);
  path.lineTo(x2, y2);
  fillStroke(canvas, path, width, color, dash);
}

void drawChevron(visage::Canvas& canvas,
                 float left_x,
                 float top_y,
                 float center_x,
                 float point_y,
                 float right_x,
                 float stroke_width,
                 uint32_t color) {
  visage::Path path;
  path.moveTo(left_x, top_y);
  path.lineTo(center_x, point_y);
  path.lineTo(right_x, top_y);
  fillStroke(canvas, path, stroke_width, color);
}

visage::Path pathFromPoints(const std::vector<SignalPoint>& points, float offset_x,
                             float offset_y) {
  visage::Path path;
  for (size_t i = 0; i < points.size(); ++i) {
    const float x = points[i].x + offset_x;
    const float y = points[i].y + offset_y;
    if (i == 0)
      path.moveTo(x, y);
    else
      path.lineTo(x, y);
  }
  return path;
}

struct BaselineLobe {
  visage::Path path;
  bool positive;
};

struct WaveformTraceStyle {
  uint32_t positive_fill_top;
  uint32_t positive_fill_bottom;
  uint32_t negative_fill_top;
  uint32_t negative_fill_bottom;
  uint32_t wide_bloom;
  uint32_t edge_bloom;
  uint32_t line;
  uint32_t hairline;
  float wide_width;
  float edge_width;
  float line_width;
  float hairline_width;
};

std::vector<BaselineLobe> signedLobesToBaseline(const std::vector<SignalPoint>& points,
                                                float baseline) {
  std::vector<BaselineLobe> lobes;
  if (points.size() < 2)
    return lobes;

  auto isPositive = [&](const SignalPoint& point) { return point.y <= baseline; };
  auto zeroCrossing = [&](const SignalPoint& a, const SignalPoint& b) {
    const float denom = b.y - a.y;
    const float t = std::abs(denom) < 0.00001f ? 0.0f : (baseline - a.y) / denom;
    return a.x + (b.x - a.x) * std::clamp(t, 0.0f, 1.0f);
  };

  visage::Path current;
  bool current_positive = isPositive(points.front());
  current.moveTo(points.front().x, baseline);
  current.lineTo(points.front().x, points.front().y);

  for (size_t i = 1; i < points.size(); ++i) {
    const SignalPoint& previous = points[i - 1];
    const SignalPoint& point = points[i];
    const bool point_positive = isPositive(point);

    if (point_positive != current_positive) {
      const float cross_x = zeroCrossing(previous, point);
      current.lineTo(cross_x, baseline);
      current.close();
      lobes.push_back({ current, current_positive });

      current = visage::Path();
      current.moveTo(cross_x, baseline);
      current.lineTo(point.x, point.y);
      current_positive = point_positive;
    }
    else {
      current.lineTo(point.x, point.y);
    }
  }

  current.lineTo(points.back().x, baseline);
  current.close();
  lobes.push_back({ current, current_positive });
  return lobes;
}

SignalPoint interpolatePoint(const SignalPoint& a, const SignalPoint& b, float target_t) {
  const float span = b.t - a.t;
  const float amount = std::abs(span) < 0.00001f ? 0.0f : (target_t - a.t) / span;
  const float clamped = std::clamp(amount, 0.0f, 1.0f);
  return { target_t,
           a.x + (b.x - a.x) * clamped,
           a.y + (b.y - a.y) * clamped,
           a.value + (b.value - a.value) * clamped };
}

std::vector<SignalPoint> waveformSegment(const std::vector<SignalPoint>& points,
                                         float start_t,
                                         float end_t) {
  std::vector<SignalPoint> segment;
  if (points.size() < 2 || end_t <= start_t)
    return segment;

  const float start = std::clamp(start_t, points.front().t, points.back().t);
  const float end = std::clamp(end_t, points.front().t, points.back().t);
  if (end <= start)
    return segment;

  for (size_t i = 1; i < points.size(); ++i) {
    const SignalPoint& previous = points[i - 1];
    const SignalPoint& point = points[i];

    if (previous.t <= start && start <= point.t)
      segment.push_back(interpolatePoint(previous, point, start));

    if (start < point.t && point.t < end)
      segment.push_back(point);

    if (previous.t <= end && end <= point.t) {
      segment.push_back(interpolatePoint(previous, point, end));
      break;
    }
  }

  return segment;
}

std::vector<SignalPoint> makeComplexAudioWaveform(const Rect& area, int samples) {
  std::vector<SignalPoint> points;
  points.reserve(static_cast<size_t>(samples));
  const float center = area.y + area.height * 0.5f;
  const float amplitude = area.height * 0.46f;

  for (int i = 0; i < samples; ++i) {
    const float t = static_cast<float>(i) / static_cast<float>(samples - 1);
    const float value = audioFileWaveformValue(t);
    points.push_back({ t, area.x + area.width * t, center - value * amplitude, value });
  }

  return points;
}

void drawGriffinWaveformTrace(DrawContext& context,
                              const std::vector<SignalPoint>& points,
                              const Rect& plot,
                              float scale,
                              const WaveformTraceStyle& style) {
  if (points.empty())
    return;

  visage::Canvas& canvas = context.canvas;
  const float center_y = plot.y + plot.height * 0.5f;
  const visage::Brush positive_fill = visage::Brush::linear(style.positive_fill_top,
                                                            style.positive_fill_bottom,
                                                            { plot.x, plot.y },
                                                            { plot.x, center_y });
  const visage::Brush negative_fill = visage::Brush::linear(style.negative_fill_top,
                                                            style.negative_fill_bottom,
                                                            { plot.x, center_y },
                                                            { plot.x, plot.y + plot.height });

  for (const BaselineLobe& lobe : signedLobesToBaseline(points, center_y)) {
    canvas.setColor(lobe.positive ? positive_fill : negative_fill);
    canvas.fill(lobe.path);
  }

  const visage::Path wave = pathFromPoints(points);
  visage::Region& wide_bloom = addBlurRegion(context, 5.5f * scale);
  drawInRegion(context, wide_bloom, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, wave, style.wide_width * scale, style.wide_bloom);
  });

  visage::Region& edge_bloom = addBlurRegion(context, 2.1f * scale);
  drawInRegion(context, edge_bloom, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, wave, style.edge_width * scale, style.edge_bloom);
  });

  visage::Region& foreground = addRegion(context, true);
  drawInRegion(context, foreground, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, wave, style.line_width * scale, style.line);
    fillStroke(region_canvas, wave, style.hairline_width * scale, style.hairline);
  });
}

[[maybe_unused]] void drawGriffinWaveformTrace(DrawContext& context,
                                               const std::vector<SignalPoint>& points,
                                               const Rect& plot,
                                               float scale) {
  constexpr WaveformTraceStyle kCanonicalWaveformTrace {
    0x98718fd8,
    0x18718fd8,
    0x18718fd8,
    0x98718fd8,
    0x58718fd8,
    0x789bbcff,
    0xffb3c5f4,
    0xe8e4ecfb,
    5.0f,
    3.1f,
    2.0f,
    0.55f,
  };
  drawGriffinWaveformTrace(context, points, plot, scale, kCanonicalWaveformTrace);
}

void drawGriffinWaveformLineTrace(DrawContext& context,
                                  const std::vector<SignalPoint>& points,
                                  float scale,
                                  const WaveformTraceStyle& style) {
  if (points.size() < 2)
    return;

  const visage::Path wave = pathFromPoints(points);
  visage::Region& wide_bloom = addBlurRegion(context, 5.5f * scale);
  drawInRegion(context, wide_bloom, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, wave, style.wide_width * scale, style.wide_bloom);
  });

  visage::Region& edge_bloom = addBlurRegion(context, 2.1f * scale);
  drawInRegion(context, edge_bloom, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, wave, style.edge_width * scale, style.edge_bloom);
  });

  visage::Region& foreground = addRegion(context, true);
  drawInRegion(context, foreground, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, wave, style.line_width * scale, style.line);
    fillStroke(region_canvas, wave, style.hairline_width * scale, style.hairline);
  });
}

void drawPlayedFutureWaveformTrace(DrawContext& context,
                                   const std::vector<SignalPoint>& points,
                                   const Rect& plot,
                                   float scale,
                                   float playhead_t,
                                   bool erase_pass) {
  (void)plot;
  constexpr WaveformTraceStyle kFutureTrace {
    0x42718fd8,
    0x0c718fd8,
    0x0c718fd8,
    0x42718fd8,
    0x345d75b8,
    0x5f89a5d0,
    0xe095a8c8,
    0xb0d8e8f4,
    4.2f,
    2.5f,
    1.55f,
    0.42f,
  };
  constexpr WaveformTraceStyle kPlayedLine {
    0x98718fd8,
    0x18718fd8,
    0x18718fd8,
    0x98718fd8,
    0x58718fd8,
    0x789bbcff,
    0xffb3c5f4,
    0xe8e4ecfb,
    5.0f,
    3.1f,
    2.0f,
    0.55f,
  };

  const std::vector<SignalPoint> played =
      erase_pass ? waveformSegment(points, playhead_t, 1.0f)
                 : waveformSegment(points, 0.0f, playhead_t);
  drawGriffinWaveformLineTrace(context, points, scale, kFutureTrace);
  drawGriffinWaveformLineTrace(context, played, scale, kPlayedLine);
}


void drawFrameCorner(visage::Canvas& canvas,
                     float x,
                     float y,
                     float horizontal,
                     float vertical,
                     int x_direction,
                     int y_direction) {
  visage::Path corner;
  corner.moveTo(x + horizontal * static_cast<float>(x_direction), y);
  corner.lineTo(x, y);
  corner.lineTo(x, y + vertical * static_cast<float>(y_direction));
  fillStroke(canvas, corner, 1.2f, 0x8897d7c5, {}, 0.0f, visage::Path::EndCap::Square);
}

void drawDiagramFrameCorners(visage::Canvas& canvas, const DiagramFrameLayout& layout) {
  const float inset = 6.0f;
  const float h = 9.5f;
  const float v = 9.5f;
  drawFrameCorner(canvas, layout.content.x + inset, layout.content.y + inset, h, v, 1, 1);
  drawFrameCorner(canvas, layout.content.x + layout.content.width - inset,
                  layout.content.y + inset, h, v, -1, 1);
  drawFrameCorner(canvas, layout.content.x + inset,
                  layout.content.y + layout.content.height - inset, h, v, 1, -1);
  drawFrameCorner(canvas, layout.content.x + layout.content.width - inset,
                  layout.content.y + layout.content.height - inset, h, v, -1, -1);
}

void drawDiagramFrame(visage::Canvas& canvas, const DiagramFrameLayout& layout) {
  canvas.setColor(0xff071016);
  canvas.roundedRectangle(layout.outer.x, layout.outer.y, layout.outer.width, layout.outer.height,
                          layout.radius);

  canvas.setColor(visage::Brush::vertical(0xff373b44, 0xff1e2124));
  canvas.roundedRectangle(layout.bevel.x, layout.bevel.y, layout.bevel.width, layout.bevel.height,
                          layout.radius - 2.0f);

  canvas.setColor(visage::Brush::vertical(0xff181d2a, 0xff0c1116));
  canvas.roundedRectangle(layout.content.x, layout.content.y, layout.content.width, layout.content.height,
                          layout.radius - 4.5f);

  canvas.setColor(0x81364c59);
  canvas.roundedRectangleBorder(layout.bevel.x, layout.bevel.y, layout.bevel.width, layout.bevel.height,
                                layout.radius - 2.0f, 1.0f);
  canvas.setColor(0x7e0a1119);
  canvas.roundedRectangleBorder(layout.content.x, layout.content.y, layout.content.width,
                                layout.content.height, layout.radius - 4.5f, 1.0f);
}

void fillRoundedRectPath(visage::Canvas& canvas,
                         float x,
                         float y,
                         float width,
                         float height,
                         float radius,
                         uint32_t color) {
  visage::Path path;
  path.addRoundedRectangle(x, y, width, height, radius);
  canvas.setColor(color);
  canvas.fill(path);
}

void fillRoundedRectPath(visage::Canvas& canvas,
                         float x,
                         float y,
                         float width,
                         float height,
                         float radius,
                         const visage::Brush& brush) {
  visage::Path path;
  path.addRoundedRectangle(x, y, width, height, radius);
  canvas.setColor(brush);
  canvas.fill(path);
}

void drawPillDashedHorizontalRule(visage::Canvas& canvas,
                                  float x,
                                  float y,
                                  float width,
                                  float thickness,
                                  float dash_length,
                                  float gap,
                                  uint32_t color) {
  visage::Path path;
  path.moveTo(x, y);
  path.lineTo(x + width, y);
  fillStroke(canvas, path, thickness, color, { dash_length, gap });
}

void drawDot(visage::Canvas& canvas, float x, float y, float radius, uint32_t color) {
  canvas.setColor(color);
  canvas.circle(x - radius, y - radius, radius * 2.0f);
}

void drawOpenDot(visage::Canvas& canvas, float x, float y, float radius, uint32_t color, float width) {
  canvas.setColor(color);
  canvas.ring(x - radius, y - radius, radius * 2.0f, width);
}

void drawTimelineGrid(visage::Canvas& canvas, const Rect& plot, std::size_t sample_count) {
  constexpr uint32_t kGridColor = 0xff18232f;
  constexpr float kGridWidth = 1.2f;

  canvas.setColor(kGridColor);
  for (size_t i = 0; i < sample_count; ++i) {
    const float t = static_cast<float>(i) / static_cast<float>(sample_count - 1);
    const float x = plot.x + plot.width * t;
    canvas.fill(x, plot.y, kGridWidth, plot.height);
  }

  constexpr int kAmplitudeRows = 4;
  for (int i = 0; i <= kAmplitudeRows; ++i) {
    const float y = plot.y + plot.height * static_cast<float>(i) / static_cast<float>(kAmplitudeRows);
    canvas.fill(plot.x, y, plot.width, kGridWidth);
  }
}

void drawTimelineZeroAxis(visage::Canvas& canvas, const Rect& plot) {
  constexpr uint32_t kAxisColor = 0xff2a3c50;
  const float center_y = plot.y + plot.height * 0.5f;
  canvas.setColor(kAxisColor);
  canvas.fill(plot.x, center_y - 0.75f, plot.width, 1.5f);
}

visage::Path ellipsePath(float center_x, float center_y, float radius_x, float radius_y) {
  visage::Path path;
  path.moveTo(center_x, center_y - radius_y);
  path.arcTo(radius_x, radius_y, 0.0f, false, true,
             visage::Point(center_x, center_y + radius_y));
  path.arcTo(radius_x, radius_y, 0.0f, false, true,
             visage::Point(center_x, center_y - radius_y));
  path.close();
  return path;
}


} // namespace adt::canonical::drawing
