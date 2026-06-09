#include "audio_diagram_tools/sampled_waveform_scene.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include <visage_graphics/post_effects.h>

namespace adt {
namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kCyclesAcrossPlot = 1.05f;
constexpr float kSineAmplitude = 0.86f;

struct Rect {
  float x = 0.0f;
  float y = 0.0f;
  float width = 0.0f;
  float height = 0.0f;
};

struct FrameLayout {
  Rect outer;
  Rect bevel;
  Rect content;
  Rect plot;
  float radius = 10.0f;
};

struct SamplePoint {
  float x = 0.0f;
  float y = 0.0f;
  float amplitude = 0.0f;
};

struct DrawContext {
  DrawContext(visage::Canvas& render_canvas, const Dimensions& render_dimensions) :
      canvas(render_canvas), dimensions(render_dimensions) { }

  visage::Canvas& canvas;
  Dimensions dimensions;
  std::vector<std::unique_ptr<visage::Region>> regions;
  std::vector<std::unique_ptr<visage::BlurPostEffect>> blur_effects;
};

Rect insetRect(const Rect& rect, float x, float y) {
  return { rect.x + x, rect.y + y, rect.width - 2.0f * x, rect.height - 2.0f * y };
}

float standardFrameGutter(const Dimensions& dimensions) {
  const float short_side = static_cast<float>(std::min(dimensions.width, dimensions.height));
  return std::clamp(short_side * 0.035f, 10.0f, 18.0f);
}

FrameLayout griffinboyFrameLayout(const Dimensions& dimensions) {
  const float width = static_cast<float>(dimensions.width);
  const float height = static_cast<float>(dimensions.height);
  const float gutter = standardFrameGutter(dimensions);
  const Rect outer { gutter, gutter, width - 2.0f * gutter, height - 2.0f * gutter };
  const Rect bevel = insetRect(outer, std::max(1.8f, width * 0.0025f),
                               std::max(1.8f, height * 0.009f));
  const Rect content = insetRect(bevel, std::max(1.7f, width * 0.0025f),
                                 std::max(1.7f, height * 0.008f));
  const float plot_x = std::max(18.0f, width * 0.027f);
  const float plot_top = std::max(13.0f, height * 0.064f);
  const float plot_bottom = std::max(10.0f, height * 0.050f);
  const Rect plot { content.x + plot_x, content.y + plot_top,
                    content.width - 2.0f * plot_x,
                    content.height - plot_top - plot_bottom };
  return { outer, bevel, content, plot, std::max(8.0f, height * 0.045f) };
}

void fillStroke(visage::Canvas& canvas,
                const visage::Path& path,
                float width,
                uint32_t color,
                visage::Path::EndCap cap = visage::Path::EndCap::Round) {
  visage::Path source = path;
  canvas.setColor(color);
  canvas.fill(source.stroke(width, visage::Path::Join::Round, cap));
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

template <typename DrawFn>
void drawInRegion(DrawContext& context, visage::Region& region, DrawFn draw) {
  context.canvas.beginRegion(&region);
  draw(context.canvas);
  context.canvas.endRegion();
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
  fillStroke(canvas, corner, 1.2f, 0x8897d7c5, visage::Path::EndCap::Square);
}

void drawFrameCorners(visage::Canvas& canvas, const FrameLayout& layout) {
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

void drawFrame(visage::Canvas& canvas, const FrameLayout& layout) {
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

SamplePoint sinePoint(const Rect& plot, float normalized_x) {
  const float x = plot.x + plot.width * normalized_x;
  const float amplitude = idealizedSineAmplitude(normalized_x);
  return { x, axisY(plot) - amplitude * amplitudeScale(plot), amplitude };
}

void appendSineBezier(visage::Path& path, const Rect& plot, float start_t, float end_t) {
  const SamplePoint start = sinePoint(plot, start_t);
  const SamplePoint end = sinePoint(plot, end_t);
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

visage::Path sineWavePath(const Rect& plot) {
  constexpr int kSegments = 32;
  const SamplePoint start = sinePoint(plot, 0.0f);

  visage::Path path;
  path.moveTo(start.x, start.y);
  appendSineRange(path, plot, 0.0f, 1.0f, kSegments);
  return path;
}

std::vector<SamplePoint> samplePoints(const Rect& plot, const EightSampleWaveformSpec& spec) {
  std::vector<SamplePoint> points;
  points.reserve(spec.amplitudes.size());

  const float denominator = static_cast<float>(spec.amplitudes.size() - 1);

  for (size_t i = 0; i < spec.amplitudes.size(); ++i) {
    const float t = static_cast<float>(i) / denominator;
    points.push_back(sinePoint(plot, t));
  }

  return points;
}

void drawZeroAxis(visage::Canvas& canvas, const Rect& plot) {
  constexpr uint32_t kAxisColor = 0xff2a3c50;
  const float center_y = axisY(plot);
  canvas.setColor(kAxisColor);
  canvas.fill(plot.x, center_y - 0.75f, plot.width, 1.5f);
}

void drawGrid(visage::Canvas& canvas, const Rect& plot, size_t sample_count) {
  constexpr uint32_t kGridColor = 0xff18232f;
  constexpr float kGridWidth = 1.0f;

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

void drawWaveFill(visage::Canvas& canvas, const Rect& plot, float center_y) {
  const visage::Brush positive_fill = visage::Brush::linear(0x98718fd8, 0x18718fd8,
                                                            { plot.x, plot.y },
                                                            { plot.x, center_y });
  const visage::Brush negative_fill = visage::Brush::linear(0x18718fd8, 0x98718fd8,
                                                            { plot.x, center_y },
                                                            { plot.x, plot.y + plot.height });

  const std::vector<float> breakpoints = sineLobeBreakpoints();
  for (size_t i = 0; i + 1 < breakpoints.size(); ++i) {
    const float start_t = breakpoints[i];
    const float end_t = breakpoints[i + 1];
    const float midpoint = (start_t + end_t) * 0.5f;
    const bool positive = idealizedSineAmplitude(midpoint) >= 0.0f;
    const SamplePoint start = sinePoint(plot, start_t);
    const SamplePoint end = sinePoint(plot, end_t);
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

void drawWaveform(DrawContext& context, const visage::Path& path) {
  visage::Region& wide_bloom = addBlurRegion(context, 5.5f);
  drawInRegion(context, wide_bloom, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, path, 5.0f, 0x58718fd8);
  });

  visage::Region& edge_bloom = addBlurRegion(context, 2.1f);
  drawInRegion(context, edge_bloom, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, path, 3.1f, 0x789bbcff);
  });

  visage::Region& foreground = addRegion(context, true);
  drawInRegion(context, foreground, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, path, 2.0f, 0xffb3c5f4);
    fillStroke(region_canvas, path, 0.55f, 0xe8e4ecfb);
  });
}

void drawSamplePoints(DrawContext& context, const std::vector<SamplePoint>& points) {
  const float radius = 6.5f;
  visage::Region& points_region = addRegion(context, true);
  drawInRegion(context, points_region, [&](visage::Canvas& region_canvas) {
    region_canvas.setColor(0xfff4f8ff);
    for (const SamplePoint& point : points)
      region_canvas.circle(point.x - radius, point.y - radius, radius * 2.0f);
  });
}

void drawEightSampleWaveform(DrawContext& context, const EightSampleWaveformSpec& spec) {
  visage::Canvas& canvas = context.canvas;
  const FrameLayout layout = griffinboyFrameLayout(context.dimensions);
  drawFrame(canvas, layout);

  const std::vector<SamplePoint> points = samplePoints(layout.plot, spec);
  const visage::Path waveform = sineWavePath(layout.plot);
  const float center_y = axisY(layout.plot);
  drawGrid(canvas, layout.plot, points.size());
  drawWaveFill(canvas, layout.plot, center_y);
  drawZeroAxis(canvas, layout.plot);
  drawWaveform(context, waveform);
  drawSamplePoints(context, points);
  drawFrameCorners(canvas, layout);
}

} // namespace

visage::Screenshot renderEightSampleWaveformFrame(const Dimensions& dimensions,
                                                  const EightSampleWaveformSpec& spec) {
  visage::Canvas canvas;
  canvas.setWindowless(dimensions.width, dimensions.height);
  canvas.updateTime(0.0);
  DrawContext context(canvas, dimensions);
  drawEightSampleWaveform(context, spec);
  canvas.submit();
  return canvas.takeScreenshot();
}

void saveEightSampleWaveformFrame(const std::string& output_path,
                                  const Dimensions& dimensions,
                                  const EightSampleWaveformSpec& spec) {
  renderEightSampleWaveformFrame(dimensions, spec).save(output_path);
}

} // namespace adt
