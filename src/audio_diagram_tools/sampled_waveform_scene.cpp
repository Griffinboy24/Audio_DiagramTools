#include "audio_diagram_tools/sampled_waveform_scene.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include <visage_graphics/post_effects.h>

namespace adt {
namespace {

constexpr float kPi = 3.14159265358979323846f;

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

struct FillSegment {
  SamplePoint start;
  SamplePoint end;
  bool positive = true;
};

struct LobeFill {
  visage::Path path;
  bool positive = true;
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

FrameLayout griffinboyFrameLayout(const Dimensions& dimensions) {
  const float width = static_cast<float>(dimensions.width);
  const float height = static_cast<float>(dimensions.height);
  const Rect outer { width * 0.0225f, height * 0.058f, width * 0.955f, height * 0.914f };
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

void drawFrame(visage::Canvas& canvas, const Dimensions& dimensions, const FrameLayout& layout) {
  canvas.setColor(0xfff4f4f9);
  canvas.fill(0, 0, dimensions.width, dimensions.height);

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
  constexpr float kCyclesAcrossPlot = 1.05f;
  constexpr float kAmplitude = 0.86f;
  return kAmplitude * std::sin(2.0f * kPi * kCyclesAcrossPlot * normalized_x);
}

std::vector<SamplePoint> samplePoints(const Rect& plot, const EightSampleWaveformSpec& spec) {
  std::vector<SamplePoint> points;
  points.reserve(spec.amplitudes.size());

  const float center_y = axisY(plot);
  const float scale = amplitudeScale(plot);
  const float denominator = static_cast<float>(spec.amplitudes.size() - 1);

  for (size_t i = 0; i < spec.amplitudes.size(); ++i) {
    const float t = static_cast<float>(i) / denominator;
    const float x = plot.x + plot.width * t;
    const float amplitude = idealizedSineAmplitude(t);
    points.push_back({ x, center_y - amplitude * scale, amplitude });
  }

  return points;
}

visage::Path waveformPath(const std::vector<SamplePoint>& points) {
  visage::Path path;
  for (size_t i = 0; i < points.size(); ++i) {
    if (i == 0)
      path.moveTo(points[i].x, points[i].y);
    else
      path.lineTo(points[i].x, points[i].y);
  }
  return path;
}

std::vector<SamplePoint> sineWavePoints(const Rect& plot, int samples) {
  std::vector<SamplePoint> points;
  samples = std::max(2, samples);
  points.reserve(static_cast<size_t>(samples));

  const float center_y = axisY(plot);
  const float scale = amplitudeScale(plot);

  for (int i = 0; i < samples; ++i) {
    const float t = static_cast<float>(i) / static_cast<float>(samples - 1);
    const float x = plot.x + plot.width * t;
    const float amplitude = idealizedSineAmplitude(t);
    points.push_back({ x, center_y - amplitude * scale, amplitude });
  }

  return points;
}

SamplePoint zeroCrossing(const SamplePoint& a, const SamplePoint& b, float center_y) {
  const float amount = std::clamp(-a.amplitude / (b.amplitude - a.amplitude), 0.0f, 1.0f);
  const float x = a.x + (b.x - a.x) * amount;
  return { x, center_y, 0.0f };
}

std::vector<FillSegment> fillSegments(const std::vector<SamplePoint>& points, float center_y) {
  std::vector<FillSegment> segments;
  if (points.size() < 2)
    return segments;

  for (size_t i = 0; i + 1 < points.size(); ++i) {
    const SamplePoint& a = points[i];
    const SamplePoint& b = points[i + 1];
    if (a.amplitude == 0.0f && b.amplitude == 0.0f)
      continue;

    if ((a.amplitude < 0.0f && b.amplitude > 0.0f) ||
        (a.amplitude > 0.0f && b.amplitude < 0.0f)) {
      const SamplePoint crossing = zeroCrossing(a, b, center_y);
      if (a.amplitude != 0.0f)
        segments.push_back({ a, crossing, a.amplitude > 0.0f });
      if (b.amplitude != 0.0f)
        segments.push_back({ crossing, b, b.amplitude > 0.0f });
    }
    else {
      const bool positive = a.amplitude == 0.0f ? b.amplitude > 0.0f : a.amplitude > 0.0f;
      segments.push_back({ a, b, positive });
    }
  }

  return segments;
}

std::vector<LobeFill> fillLobes(const std::vector<SamplePoint>& points, float center_y) {
  std::vector<LobeFill> lobes;
  const std::vector<FillSegment> segments = fillSegments(points, center_y);
  if (segments.empty())
    return lobes;

  visage::Path current;
  bool open = false;
  bool positive = segments.front().positive;
  SamplePoint last_point;

  auto closeCurrent = [&]() {
    current.lineTo(last_point.x, center_y);
    current.close();
    lobes.push_back({ current, positive });
  };

  for (const FillSegment& segment : segments) {
    if (!open || segment.positive != positive || std::abs(segment.start.x - last_point.x) > 0.01f) {
      if (open)
        closeCurrent();

      current = visage::Path();
      positive = segment.positive;
      current.moveTo(segment.start.x, center_y);
      current.lineTo(segment.start.x, segment.start.y);
      open = true;
    }

    current.lineTo(segment.end.x, segment.end.y);
    last_point = segment.end;
  }

  if (open)
    closeCurrent();

  return lobes;
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

void drawWaveFill(visage::Canvas& canvas, const std::vector<SamplePoint>& points, float center_y) {
  for (const LobeFill& fill : fillLobes(points, center_y)) {
    if (fill.positive)
      canvas.setColor(visage::Brush::vertical(0x98718fd8, 0x00718fd8));
    else
      canvas.setColor(visage::Brush::vertical(0x00718fd8, 0x98718fd8));
    canvas.fill(fill.path);
  }
}

void drawWaveform(DrawContext& context, const visage::Path& path) {
  visage::Region& wide_bloom = addBlurRegion(context, 2.7f);
  drawInRegion(context, wide_bloom, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, path, 3.4f, 0x3f789dff);
  });

  visage::Region& edge_bloom = addBlurRegion(context, 1.35f);
  drawInRegion(context, edge_bloom, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, path, 2.4f, 0x5c9fc0ff);
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
  drawFrame(canvas, context.dimensions, layout);

  const std::vector<SamplePoint> points = samplePoints(layout.plot, spec);
  const std::vector<SamplePoint> smooth_points = sineWavePoints(layout.plot, 900);
  const std::vector<SamplePoint> fill_points = sineWavePoints(layout.plot, 260);
  const float center_y = axisY(layout.plot);
  drawGrid(canvas, layout.plot, points.size());
  drawWaveFill(canvas, fill_points, center_y);
  drawZeroAxis(canvas, layout.plot);
  drawWaveform(context, waveformPath(smooth_points));
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
