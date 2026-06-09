#include "audio_diagram_tools/style_lab_scene.h"

#include "audio_diagram_tools/png_export.h"

#include <array>
#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <embedded/fonts.h>
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

struct SignalPoint {
  float t = 0.0f;
  float x = 0.0f;
  float y = 0.0f;
  float value = 0.0f;
};

struct DrawContext {
  DrawContext(visage::Canvas& render_canvas, const Dimensions& render_dimensions) :
      canvas(render_canvas), dimensions(render_dimensions) { }

  visage::Canvas& canvas;
  Dimensions dimensions;
  std::vector<std::unique_ptr<visage::Region>> regions;
  std::vector<std::unique_ptr<visage::BlurPostEffect>> blur_effects;
};

const std::array<StyleStudy, 19> kStyleStudies = {
  StyleStudy { "warm-scope", "Warm oscilloscope overlay with solid/dashed phase traces" },
  StyleStudy { "spectral-callout", "Dense spectral trace with a restrained callout treatment" },
  StyleStudy { "blue-decay", "Soft blue chirp trace with bloom and sparse scale markers" },
  StyleStudy { "quiet-overlay", "Muted educational overlay with gentle grid and low-contrast secondary trace" },
  StyleStudy { "filled-ribbon", "Filled waveform silhouette with lit edge and internal shading" },
  StyleStudy { "blue-area", "Single-cycle blue area fill with a restrained luminous edge" },
  StyleStudy { "cream-markers", "Cream trace with sparse rose control markers and dotted guides" },
  StyleStudy { "paper-ink", "Light paper plot with ink primary trace and red dashed secondary trace" },
  StyleStudy { "mono-chirp", "Monochrome chirp trace with no colored glow" },
  StyleStudy { "blue-ridge", "Filled blue spectral ridge with bloom, grid, and peak marker" },
  StyleStudy { "blue-ridge-framed", "Blue ridge study inside a reusable rounded diagram frame" },
  StyleStudy { "sample-table-card", "Pale text and table card with Griffinboy-compatible rounded geometry" },
  StyleStudy { "sample-table-card-page", "Higher-contrast table card variant for white article pages" },
  StyleStudy { "sample-table-card-paper", "Warm paper table card with a tight editorial shadow" },
  StyleStudy { "sample-table-card-tight", "Near-white table card with crisp tight page shadow" },
  StyleStudy { "sample-table-card-dark-band", "Dark information band table card with a violet accent strip" },
  StyleStudy { "sample-table-card-paper-ink", "Warm paper/ink table card with dotted plot-style rules" },
  StyleStudy { "sample-table-card-schematic", "Crisp white schematic card with tight shadow and line-art badge" },
  StyleStudy { "sample-table-card-tech-band", "Dark technical table band with violet strip and geometric details" },
};

float clamp01(float value) {
  return std::max(0.0f, std::min(1.0f, value));
}

float smoothstep(float edge0, float edge1, float value) {
  const float t = clamp01((value - edge0) / (edge1 - edge0));
  return t * t * (3.0f - 2.0f * t);
}

float gaussian(float value, float center, float width) {
  const float distance = (value - center) / width;
  return std::exp(-distance * distance);
}

uint32_t alphaColor(uint8_t alpha, uint32_t rgb) {
  return (static_cast<uint32_t>(alpha) << 24) | (rgb & 0x00ffffffu);
}

bool hasAlpha(uint32_t color) {
  return (color >> 24) != 0;
}

Rect studyArea(const Dimensions& dimensions, float left = 64.0f, float top = 52.0f,
               float right = 64.0f, float bottom = 58.0f) {
  return { left, top, static_cast<float>(dimensions.width) - left - right,
           static_cast<float>(dimensions.height) - top - bottom };
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
                const std::vector<float>& dash = {},
                float dash_offset = 0.0f,
                visage::Path::EndCap cap = visage::Path::EndCap::Round) {
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

template <typename DrawFn>
void drawInRegion(DrawContext& context, visage::Region& region, DrawFn draw) {
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
              const std::vector<float>& dash = {}) {
  visage::Path path;
  path.moveTo(x1, y1);
  path.lineTo(x2, y2);
  fillStroke(canvas, path, width, color, dash);
}

void drawArrow(visage::Canvas& canvas,
               float x1,
               float y1,
               float x2,
               float y2,
               uint32_t color) {
  drawLine(canvas, x1, y1, x2, y2, 2.0f, color);

  const float angle = std::atan2(y2 - y1, x2 - x1);
  const float size = 12.0f;
  const float wing = 0.55f;
  visage::Path head;
  head.moveTo(x2, y2);
  head.lineTo(x2 - std::cos(angle - wing) * size, y2 - std::sin(angle - wing) * size);
  head.lineTo(x2 - std::cos(angle + wing) * size, y2 - std::sin(angle + wing) * size);
  head.close();
  canvas.setColor(color);
  canvas.fill(head);
}

void drawBackground(visage::Canvas& canvas,
                    const Dimensions& dimensions,
                    uint32_t top,
                    uint32_t bottom,
                    uint32_t accent,
                    float accent_x = 0.78f,
                    float accent_y = 0.28f) {
  canvas.setColor(visage::Brush::vertical(top, bottom));
  canvas.fill(0, 0, dimensions.width, dimensions.height);

  visage::Gradient accent_gradient(accent, 0x00000000);
  canvas.setColor(visage::Brush::radial(accent_gradient,
                                        visage::Point(dimensions.width * accent_x,
                                                      dimensions.height * accent_y),
                                        dimensions.height * 0.78f));
  canvas.fill(0, 0, dimensions.width, dimensions.height);
}

void drawGrid(visage::Canvas& canvas,
              const Rect& area,
              int columns,
              int rows,
              int major_every,
              uint32_t minor,
              uint32_t major,
              uint32_t border = 0x00000000) {
  for (int i = 0; i <= columns; ++i) {
    const bool is_major = major_every > 0 && i % major_every == 0;
    const float x = area.x + area.width * static_cast<float>(i) / static_cast<float>(columns);
    canvas.setColor(is_major ? major : minor);
    canvas.fill(x, area.y, is_major ? 1.4f : 1.0f, area.height);
  }

  for (int i = 0; i <= rows; ++i) {
    const bool is_major = major_every > 0 && i % major_every == 0;
    const float y = area.y + area.height * static_cast<float>(i) / static_cast<float>(rows);
    canvas.setColor(is_major ? major : minor);
    canvas.fill(area.x, y, area.width, is_major ? 1.4f : 1.0f);
  }

  if ((border >> 24) != 0) {
    canvas.setColor(border);
    canvas.rectangleBorder(area.x, area.y, area.width, area.height, 1.0f);
  }
}

float baseEnvelope(float t) {
  const float slow = 0.68f + 0.32f * std::sin(2.0f * kPi * (t * 0.92f + 0.05f));
  const float right_fade = 1.0f - 0.68f * smoothstep(0.58f, 1.0f, t);
  const float left_ease = smoothstep(0.02f, 0.11f, t);
  return slow * right_fade * left_ease;
}

std::vector<SignalPoint> makeChirp(const Rect& area, int samples, float phase_cycles = 0.0f) {
  std::vector<SignalPoint> points;
  points.reserve(static_cast<size_t>(samples));
  const float center = area.y + area.height * 0.54f;
  const float amplitude = area.height * 0.37f;

  for (int i = 0; i < samples; ++i) {
    const float t = static_cast<float>(i) / static_cast<float>(samples - 1);
    const float cycles = 1.05f * t + 12.5f * t * t;
    const float value = std::sin(2.0f * kPi * (cycles + phase_cycles)) * baseEnvelope(t);
    points.push_back({ t, area.x + t * area.width, center - value * amplitude, value });
  }

  return points;
}

std::vector<SignalPoint> makePhaseSine(const Rect& area,
                                       int samples,
                                       float phase_cycles,
                                       float cycles = 3.15f,
                                       float vertical_scale = 0.36f) {
  std::vector<SignalPoint> points;
  points.reserve(static_cast<size_t>(samples));
  const float center = area.y + area.height * 0.52f;
  const float amplitude = area.height * vertical_scale;

  for (int i = 0; i < samples; ++i) {
    const float t = static_cast<float>(i) / static_cast<float>(samples - 1);
    const float edge = smoothstep(0.0f, 0.03f, t) * (1.0f - smoothstep(0.97f, 1.0f, t));
    const float value = std::sin(2.0f * kPi * (cycles * t + phase_cycles)) * edge;
    points.push_back({ t, area.x + t * area.width, center - value * amplitude, value });
  }

  return points;
}

visage::Path pathFromPoints(const std::vector<SignalPoint>& points, float offset_x = 0.0f,
                            float offset_y = 0.0f) {
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

visage::Path fillToBaseline(const std::vector<SignalPoint>& points, float baseline) {
  visage::Path path;
  if (points.empty())
    return path;

  path.moveTo(points.front().x, baseline);
  for (const SignalPoint& point : points)
    path.lineTo(point.x, point.y);
  path.lineTo(points.back().x, baseline);
  path.close();
  return path;
}

std::vector<visage::Path> fillLobesToBaseline(const std::vector<SignalPoint>& points, float baseline) {
  std::vector<visage::Path> paths;
  if (points.size() < 2)
    return paths;

  visage::Path current;
  bool open = false;
  bool positive = false;

  for (const SignalPoint& point : points) {
    const bool point_positive = point.y <= baseline;
    if (!open) {
      current.moveTo(point.x, baseline);
      current.lineTo(point.x, point.y);
      open = true;
      positive = point_positive;
      continue;
    }

    if (point_positive != positive) {
      current.lineTo(point.x, baseline);
      current.close();
      paths.push_back(current);

      current = visage::Path();
      current.moveTo(point.x, baseline);
      current.lineTo(point.x, point.y);
      positive = point_positive;
    }
    else {
      current.lineTo(point.x, point.y);
    }
  }

  current.lineTo(points.back().x, baseline);
  current.close();
  paths.push_back(current);
  return paths;
}

void drawWarmTrace(visage::Canvas& canvas,
                   const visage::Path& path,
                   bool dashed,
                   bool foreground) {
  const std::vector<float> dash = dashed ? std::vector<float> { 10.0f, 9.0f } : std::vector<float> {};
  const uint32_t line = foreground ? 0xffff705c : 0xccb84b42;
  const float width = foreground ? 3.2f : 2.2f;

  fillStroke(canvas, path, width + 10.0f, foreground ? 0x38ff5f4f : 0x20ff5f4f, dash);
  fillStroke(canvas, path, width + 2.0f, foreground ? 0x9aff705c : 0x72b84b42, dash);
  fillStroke(canvas, path, width, line, dash);
}

void drawSpectralTrace(visage::Canvas& canvas, const visage::Path& path) {
  fillStroke(canvas, path, 4.8f, 0x555c546a);
  fillStroke(canvas, path, 2.4f, 0xffd2cedb);
}

void drawBlueBloomTrace(visage::Canvas& canvas, const visage::Path& path) {
  fillStroke(canvas, path, 18.0f, 0x226fa0ff);
  fillStroke(canvas, path, 7.0f, 0x667fa8ee);
  fillStroke(canvas, path, 2.4f, 0xff9bbcff);
}

void drawQuietTrace(visage::Canvas& canvas,
                    const visage::Path& path,
                    uint32_t color,
                    float width,
                    const std::vector<float>& dash = {}) {
  fillStroke(canvas, path, width, color, dash);
}

void drawRibbonEdge(visage::Canvas& canvas, const visage::Path& path) {
  fillStroke(canvas, path, 6.0f, 0x385f7eb8);
  fillStroke(canvas, path, 2.2f, 0xff9fb9ed);
}

void drawWarmScope(visage::Canvas& canvas, const Dimensions& dimensions) {
  drawBackground(canvas, dimensions, 0xff111111, 0xff0a0b0b, 0x22ff553d, 0.18f, 0.35f);
  const Rect area = studyArea(dimensions, 58.0f, 64.0f, 58.0f, 74.0f);
  drawGrid(canvas, area, 30, 7, 5, 0x153e2926, 0x2a6d342d, 0x44784237);

  const float axis_y = area.y + area.height * 0.52f;
  drawLine(canvas, area.x, axis_y, area.x + area.width, axis_y, 1.6f, 0x55c94d3f);
  drawLine(canvas, area.x, area.y + area.height, area.x + area.width, area.y + area.height,
           2.0f, 0x99c94d3f);
  canvas.setColor(0xffd95d4c);
  canvas.triangleRight(area.x + area.width - 5.0f, area.y + area.height - 6.0f, 7.0f);

  const auto input = makePhaseSine(area, 900, 0.0f);
  const auto shifted = makePhaseSine(area, 900, -0.22f);
  const visage::Path input_path = pathFromPoints(input);
  const visage::Path shifted_path = pathFromPoints(shifted);
  drawWarmTrace(canvas, shifted_path, true, false);
  drawWarmTrace(canvas, input_path, false, true);

  text(canvas, "Phase shift through a crossover", 21.0f, 0xffff6b58,
       visage::Font::kTopLeft, area.x, 18.0f, 520.0f, 34.0f);
  text(canvas, "Original", 17.0f, 0xffff7b68, visage::Font::kTopLeft,
       area.x + area.width - 190.0f, 20.0f, 170.0f, 26.0f);
  text(canvas, "After filter", 17.0f, 0xffe05f50, visage::Font::kTopLeft,
       area.x + area.width - 190.0f, 52.0f, 170.0f, 26.0f);
  drawLine(canvas, area.x + area.width - 250.0f, 30.0f, area.x + area.width - 205.0f, 30.0f,
           3.0f, 0xffff705c);
  drawLine(canvas, area.x + area.width - 250.0f, 63.0f, area.x + area.width - 205.0f, 63.0f,
           2.5f, 0xffd95648, { 10.0f, 7.0f });
  drawArrow(canvas, area.x + area.width * 0.84f, axis_y + 50.0f,
            area.x + area.width * 0.875f, axis_y + 25.0f, 0xffff705c);
  text(canvas, "phase offset", 18.0f, 0xffff705c, visage::Font::kTopLeft,
       area.x + area.width * 0.88f, axis_y + 26.0f, 160.0f, 28.0f);
  text(canvas, "Time", 18.0f, 0xffff705c, visage::Font::kBottomLeft,
       area.x, area.y + area.height + 12.0f, 120.0f, 28.0f);
}

void drawSpectralCallout(visage::Canvas& canvas, const Dimensions& dimensions) {
  drawBackground(canvas, dimensions, 0xff101019, 0xff090a10, 0x442c155a, 0.76f, 0.26f);
  const Rect area = studyArea(dimensions, 72.0f, 58.0f, 70.0f, 58.0f);
  drawGrid(canvas, area, 42, 6, 7, 0x182d3141, 0x343b4258, 0x55665391);

  const auto chirp = makeChirp(area, 1400);
  const visage::Path wave = pathFromPoints(chirp);
  const float baseline = area.y + area.height * 0.54f;
  canvas.setColor(visage::Brush::vertical(0x22ffffff, 0x001a1428));
  canvas.fill(fillToBaseline(chirp, baseline));

  for (int i = 0; i < 95; ++i) {
    const float t = static_cast<float>(i) / 94.0f;
    const float x = area.x + t * area.width;
    const float h = area.height * (0.12f + 0.45f * std::abs(std::sin(22.0f * t + t * t * 34.0f))) *
                    (0.35f + 0.65f * smoothstep(0.18f, 0.88f, t));
    canvas.setColor(alphaColor(static_cast<uint8_t>(26 + 58 * smoothstep(0.4f, 0.95f, t)), 0xb9a7d9));
    canvas.fill(x, baseline - h * 0.5f, 1.0f, h);
  }

  drawSpectralTrace(canvas, wave);

  const float callout_x = area.x + area.width * 0.61f;
  const float callout_y = baseline - area.height * 0.08f;
  canvas.setColor(0x44a549ff);
  canvas.ring(callout_x - 15.0f, callout_y - 15.0f, 30.0f, 4.0f);
  drawLine(canvas, callout_x + 18.0f, callout_y - 12.0f,
           callout_x + 115.0f, callout_y - 90.0f, 2.0f, 0xff9b4dff);
  canvas.setColor(0x229b4dff);
  canvas.roundedRectangle(callout_x + 115.0f, callout_y - 110.0f, 238.0f, 42.0f, 4.0f);
  canvas.setColor(0xff9b4dff);
  canvas.roundedRectangleBorder(callout_x + 115.0f, callout_y - 110.0f, 238.0f, 42.0f, 4.0f, 1.5f);
  text(canvas, "phase cancellation", 17.0f, 0xffb56dff, visage::Font::kCenter,
       callout_x + 128.0f, callout_y - 104.0f, 210.0f, 30.0f);

  text(canvas, "SPECTRUM (MAGNITUDE)", 15.0f, 0xffc8c6d2, visage::Font::kTopLeft,
       area.x + 12.0f, area.y - 34.0f, 260.0f, 26.0f);
  text(canvas, "100 Hz", 16.0f, 0xffc8c6d2, visage::Font::kTopLeft,
       area.x + area.width * 0.17f, area.y + 6.0f, 100.0f, 24.0f);
  text(canvas, "1 kHz", 16.0f, 0xffc8c6d2, visage::Font::kTopLeft,
       area.x + area.width * 0.46f, area.y + 6.0f, 100.0f, 24.0f);
  text(canvas, "10 kHz", 16.0f, 0xffc8c6d2, visage::Font::kTopLeft,
       area.x + area.width * 0.74f, area.y + 6.0f, 100.0f, 24.0f);
}

void drawBlueDecay(visage::Canvas& canvas, const Dimensions& dimensions) {
  drawBackground(canvas, dimensions, 0xff111820, 0xff0a0d12, 0x1f5d85d4, 0.78f, 0.48f);
  const Rect area = studyArea(dimensions, 42.0f, 48.0f, 44.0f, 78.0f);
  drawGrid(canvas, area, 8, 3, 2, 0x133c4c5f, 0x2a536172);

  const auto chirp = makeChirp(area, 1600, -0.08f);
  const visage::Path wave = pathFromPoints(chirp);
  drawBlueBloomTrace(canvas, wave);

  const float center = area.y + area.height * 0.54f;
  canvas.setColor(visage::Brush::linear(visage::Gradient(0x00000000, 0x266ca8ff, 0x00000000),
                                        visage::Point(area.x + area.width * 0.44f, center),
                                        visage::Point(area.x + area.width, center)));
  canvas.fill(area.x + area.width * 0.42f, center - 46.0f, area.width * 0.58f, 92.0f);

  text(canvas, "100 Hz", 17.0f, 0xff9ca8b7, visage::Font::kCenter,
       area.x + area.width * 0.14f - 45.0f, area.y + area.height + 18.0f, 90.0f, 24.0f);
  text(canvas, "1 kHz", 17.0f, 0xff9ca8b7, visage::Font::kCenter,
       area.x + area.width * 0.40f - 45.0f, area.y + area.height + 18.0f, 90.0f, 24.0f);
  text(canvas, "10 kHz", 17.0f, 0xff9ca8b7, visage::Font::kCenter,
       area.x + area.width * 0.73f - 45.0f, area.y + area.height + 18.0f, 90.0f, 24.0f);
}

void drawQuietOverlay(visage::Canvas& canvas, const Dimensions& dimensions) {
  drawBackground(canvas, dimensions, 0xff27313d, 0xff1d252f, 0x223d5f87, 0.36f, 0.36f);
  const Rect area = studyArea(dimensions, 28.0f, 28.0f, 28.0f, 36.0f);
  drawGrid(canvas, area, 16, 8, 4, 0x18495b6b, 0x405f7180, 0x3e71869a);

  const auto a = makePhaseSine(area, 850, 0.04f, 1.48f, 0.32f);
  const auto b = makePhaseSine(area, 850, 0.34f, 1.48f, 0.32f);
  const visage::Path primary = pathFromPoints(a);
  const visage::Path secondary = pathFromPoints(b);
  const float axis_y = area.y + area.height * 0.52f;
  drawLine(canvas, area.x, axis_y, area.x + area.width, axis_y, 1.4f, 0x7092a2b0, { 8.0f, 5.0f });
  drawQuietTrace(canvas, secondary, 0xa0a9b2bd, 2.0f, { 8.0f, 7.0f });
  drawQuietTrace(canvas, primary, 0xff96bfff, 2.6f);

  canvas.setColor(0x338fb5ff);
  canvas.fill(fillToBaseline(a, axis_y));
}

void drawFilledRibbon(visage::Canvas& canvas, const Dimensions& dimensions) {
  drawBackground(canvas, dimensions, 0xff101923, 0xff0b1018, 0x173b5d94, 0.62f, 0.28f);
  const Rect area = studyArea(dimensions, 42.0f, 40.0f, 42.0f, 54.0f);
  drawGrid(canvas, area, 10, 5, 2, 0x112c3b4d, 0x243e5168);

  std::vector<SignalPoint> points;
  points.reserve(1400);
  const float baseline = area.y + area.height * 0.80f;
  for (int i = 0; i < 1400; ++i) {
    const float t = static_cast<float>(i) / 1399.0f;
    const float left_bloom = std::exp(-std::pow((t - 0.10f) / 0.06f, 2.0f));
    const float chirp = std::abs(std::sin(2.0f * kPi * (1.2f * t + 22.0f * t * t)));
    const float chirp_env = std::exp(-std::pow((t - 0.46f) / 0.18f, 2.0f)) * chirp;
    const float right_hill = 0.34f * std::exp(-std::pow((t - 0.77f) / 0.19f, 2.0f));
    const float value = std::min(0.98f, 0.06f + 0.72f * left_bloom + 0.60f * chirp_env + right_hill);
    const float x = area.x + t * area.width;
    const float y = baseline - value * area.height * 0.74f;
    points.push_back({ t, x, y, value });
  }

  const visage::Path ribbon = fillToBaseline(points, baseline);
  canvas.setColor(visage::Brush::vertical(0x88a9c2ff, 0x18213968));
  canvas.fill(ribbon);
  const visage::Path crest = pathFromPoints(points);
  drawRibbonEdge(canvas, crest);
}

std::vector<SignalPoint> makeSpectralRidge(const Rect& area, int samples) {
  std::vector<SignalPoint> points;
  points.reserve(static_cast<size_t>(samples));

  const float baseline = area.y + area.height * 0.90f;
  const float height = area.height * 0.80f;
  struct Peak {
    float center;
    float width;
    float amplitude;
  };

  const std::array<Peak, 18> peaks = {
    Peak { 0.205f, 0.075f, 0.96f },
    Peak { 0.301f, 0.028f, 0.70f },
    Peak { 0.365f, 0.020f, 0.65f },
    Peak { 0.407f, 0.016f, 0.61f },
    Peak { 0.440f, 0.014f, 0.58f },
    Peak { 0.468f, 0.012f, 0.55f },
    Peak { 0.492f, 0.011f, 0.52f },
    Peak { 0.514f, 0.010f, 0.49f },
    Peak { 0.534f, 0.009f, 0.46f },
    Peak { 0.552f, 0.008f, 0.43f },
    Peak { 0.569f, 0.0075f, 0.40f },
    Peak { 0.585f, 0.0070f, 0.37f },
    Peak { 0.600f, 0.0065f, 0.34f },
    Peak { 0.614f, 0.0060f, 0.31f },
    Peak { 0.628f, 0.0058f, 0.28f },
    Peak { 0.641f, 0.0056f, 0.25f },
    Peak { 0.653f, 0.0054f, 0.22f },
    Peak { 0.664f, 0.0052f, 0.19f },
  };

  for (int i = 0; i < samples; ++i) {
    const float t = static_cast<float>(i) / static_cast<float>(samples - 1);
    const float fade_in = smoothstep(0.01f, 0.15f, t);
    float value = 0.016f;

    for (const Peak& peak : peaks)
      value = std::max(value, peak.amplitude * gaussian(t, peak.center, peak.width));

    const float lobe_floor = (0.026f + 0.040f * smoothstep(0.34f, 0.55f, t)) *
                             (1.0f - smoothstep(0.62f, 0.70f, t));
    value = std::max(value, lobe_floor);

    const float right_gate = smoothstep(0.640f, 0.720f, t);
    const float right_value = right_gate * (0.13f + 0.27f * gaussian(t, 0.755f, 0.215f));
    value = std::max(value, right_value);
    value = clamp01(value * fade_in);

    const float x = area.x + t * area.width;
    const float y = baseline - value * height;
    points.push_back({ t, x, y, value });
  }

  return points;
}

void drawBlueRidgeGrid(visage::Canvas& canvas, const Rect& area) {
  constexpr uint32_t kGridColor = 0xff18232f;
  constexpr float kGridWidth = 1.2f;

  canvas.setColor(kGridColor);
  canvas.fill(area.x, area.y, kGridWidth, area.height);
  canvas.fill(area.x + area.width - kGridWidth, area.y, kGridWidth, area.height);

  const float vertical_spacing = area.width / 11.0f;
  for (int i = 1; i < 11; ++i) {
    const float x = area.x + vertical_spacing * static_cast<float>(i);
    if (x > area.x + area.width)
      break;

    canvas.fill(x, area.y, kGridWidth, area.height);
  }

  const float horizontal_spacing = area.height / 6.0f;
  const int rows = 6;
  for (int i = 0; i <= rows; ++i) {
    const float y = area.y + horizontal_spacing * static_cast<float>(i);
    if (y > area.y + area.height)
      break;

    canvas.fill(area.x, y, area.width, kGridWidth);
  }
}

void drawBlueRidgeBackground(visage::Canvas& canvas,
                             const Dimensions& dimensions,
                             const Rect& area,
                             bool fill_canvas) {
  canvas.setColor(visage::Brush::vertical(0xff181d2a, 0xff0c1116));
  if (fill_canvas)
    canvas.fill(0, 0, dimensions.width, dimensions.height);
  else
    canvas.fill(area.x, area.y, area.width, area.height);
}

void fillRidgeArea(visage::Canvas& canvas,
                   const std::vector<SignalPoint>& ridge,
                   float top,
                   float bottom) {
  if (ridge.size() < 2)
    return;

  const float extended_bottom = bottom + std::max(8.0f, (bottom - top) * 0.08f);
  canvas.setColor(visage::Brush::linear(0xa8718fd8, 0x00718fd8,
                                        { 0.0f, top },
                                        { 0.0f, extended_bottom }));
  canvas.fill(fillToBaseline(ridge, extended_bottom));
}

void drawBlueRidgePlot(DrawContext& context,
                       const Dimensions& dimensions,
                       const Rect& area,
                       bool fill_canvas_background,
                       bool fill_plot_background = true,
                       bool show_peak_marker = true) {
  visage::Canvas& canvas = context.canvas;
  if (fill_canvas_background || fill_plot_background)
    drawBlueRidgeBackground(canvas, dimensions, area, fill_canvas_background);

  const auto ridge = makeSpectralRidge(area, 1800);
  const auto fill_ridge = makeSpectralRidge(area, 260);
  const float bottom = area.y + area.height;
  const visage::Path crest = pathFromPoints(ridge);

  drawBlueRidgeGrid(canvas, area);
  fillRidgeArea(canvas, fill_ridge, area.y, bottom);

  visage::Region& wide_bloom = addBlurRegion(context, 5.5f);
  drawInRegion(context, wide_bloom, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, crest, 5.0f, 0x58718fd8);
  });

  visage::Region& edge_bloom = addBlurRegion(context, 2.1f);
  drawInRegion(context, edge_bloom, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, crest, 3.1f, 0x789bbcff);
  });

  visage::Region& foreground = addRegion(context, true);
  drawInRegion(context, foreground, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, crest, 2.0f, 0xffb3c5f4);
    fillStroke(region_canvas, crest, 0.55f, 0xe8e4ecfb);

    if (show_peak_marker) {
      const auto peak = std::max_element(ridge.begin(), ridge.end(), [](const SignalPoint& a,
                                                                        const SignalPoint& b) {
        return a.value < b.value;
      });
      if (peak != ridge.end()) {
        region_canvas.setColor(0xfff4f8ff);
        region_canvas.circle(peak->x - 3.0f, peak->y - 3.0f, 6.0f);
      }
    }
  });
}

void drawBlueRidge(DrawContext& context, const Dimensions& dimensions) {
  const Rect area = studyArea(dimensions, 0.0f, 0.0f, 0.0f,
                              static_cast<float>(dimensions.height) * 0.07f);
  drawBlueRidgePlot(context, dimensions, area, true);
}

struct DiagramFrameLayout {
  Rect outer;
  Rect bevel;
  Rect content;
  Rect plot;
  float radius = 10.0f;
};

float standardFrameGutter(const Dimensions& dimensions) {
  const float short_side = static_cast<float>(std::min(dimensions.width, dimensions.height));
  return std::clamp(short_side * 0.035f, 10.0f, 18.0f);
}

DiagramFrameLayout diagramFrameLayout(const Dimensions& dimensions) {
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

void drawBlueRidgeFramed(DrawContext& context, const Dimensions& dimensions) {
  DiagramFrameLayout layout = diagramFrameLayout(dimensions);
  drawDiagramFrame(context.canvas, layout);
  drawBlueRidgePlot(context, dimensions, layout.plot, false, false, false);
  drawDiagramFrameCorners(context.canvas, layout);
}

struct SampleTableCardLayout {
  Rect card;
  Rect table;
  Rect copy;
  float radius = 10.0f;
  float badge_radius = 20.0f;
  float badge_x = 0.0f;
  float badge_y = 0.0f;
};

enum class SampleCardSurface {
  Solid,
  VerticalGradient,
};

enum class SampleTableRules {
  Solid,
  PaperDotted,
  SchematicLines,
  TechCells,
};

enum class SampleBadgeTreatment {
  Filled,
  InkFilled,
  LightOutline,
  DarkOutline,
};

struct SampleTableCardStyle {
  uint32_t card_face = 0xffffffff;
  uint32_t card_border = 0xffe8ebf0;
  uint32_t table_fill = 0xfffcfcfd;
  uint32_t table_line = 0xffdce0e2;
  uint32_t title_text = 0xff111827;
  uint32_t body_text = 0xff718096;
  uint32_t value_text = 0xff2f3947;
  uint32_t badge_top = 0xff17233a;
  uint32_t badge_bottom = 0xff08111f;
  uint32_t badge_text = 0xffffffff;
  uint32_t badge_shadow = 0x34081527;
  uint32_t badge_ring = 0x661f2d46;
  uint32_t shadow_near = 0x10081527;
  uint32_t shadow_far = 0x080b1728;
  uint32_t accent_band = 0x00000000;
  float card_border_width = 3.0f;
  float table_border_width = 1.5f;
  float table_divider_width = 1.1f;
  float shadow_y_offset = -13.0f;
  float shadow_clamp_offset = 0.25f;
  float shadow_near_height = 24.0f;
  float shadow_near_blur = 34.0f;
  float shadow_far_inset = 34.0f;
  float shadow_far_y_offset = 8.0f;
  float shadow_far_height = 18.0f;
  float shadow_far_blur = 46.0f;
  float accent_band_width = 0.0f;
  uint32_t card_face_bottom = 0xffffffff;
  uint32_t table_fill_bottom = 0xfffcfcfd;
  uint32_t accent_line = 0x00000000;
  uint32_t detail_line = 0x00000000;
  uint32_t detail_dot = 0x00000000;
  float badge_shadow_offset = 5.0f;
  float badge_shadow_blur = 16.0f;
  float table_rounding_scale = 0.34f;
  SampleCardSurface card_surface = SampleCardSurface::Solid;
  SampleTableRules table_rules = SampleTableRules::Solid;
  SampleBadgeTreatment badge_treatment = SampleBadgeTreatment::Filled;
};

constexpr SampleTableCardStyle kSampleTableCardStyle {};
constexpr SampleTableCardStyle kSampleTableCardPageStyle {
  0xffffffff, // card_face
  0xffd8dde6, // card_border
  0xfffcfcfd, // table_fill
  0xffcbd3dc, // table_line
  0xff0f1724, // title_text
  0xff5f6d82, // body_text
  0xff253142, // value_text
  0xff17233a, // badge_top
  0xff08111f, // badge_bottom
  0xffffffff, // badge_text
  0x30081527, // badge_shadow
  0x5c1f2d46, // badge_ring
  0x0d081527, // shadow_near
  0x060b1728, // shadow_far
  0x00000000, // accent_band
  3.0f, // card_border_width
  1.45f, // table_border_width
  1.15f, // table_divider_width
  -16.0f, // shadow_y_offset
  -1.75f, // shadow_clamp_offset
  24.0f, // shadow_near_height
  34.0f, // shadow_near_blur
  34.0f, // shadow_far_inset
  8.0f, // shadow_far_y_offset
  18.0f, // shadow_far_height
  46.0f, // shadow_far_blur
  0.0f, // accent_band_width
};

constexpr SampleTableCardStyle kSampleTableCardPaperStyle {
  0xfffffcf6, // card_face
  0xffddd8cf, // card_border
  0xfffffdf9, // table_fill
  0xffd8d1c7, // table_line
  0xff242321, // title_text
  0xff766d62, // body_text
  0xff34302b, // value_text
  0xff252525, // badge_top
  0xff121212, // badge_bottom
  0xfffffcf6, // badge_text
  0x26000000, // badge_shadow
  0x44333333, // badge_ring
  0x18000000, // shadow_near
  0x07000000, // shadow_far
  0x00000000, // accent_band
  2.0f, // card_border_width
  1.25f, // table_border_width
  1.05f, // table_divider_width
  -9.0f, // shadow_y_offset
  -1.0f, // shadow_clamp_offset
  16.0f, // shadow_near_height
  20.0f, // shadow_near_blur
  42.0f, // shadow_far_inset
  7.0f, // shadow_far_y_offset
  12.0f, // shadow_far_height
  30.0f, // shadow_far_blur
  0.0f, // accent_band_width
};

constexpr SampleTableCardStyle kSampleTableCardTightStyle {
  0xffffffff, // card_face
  0xffdfe5ec, // card_border
  0xfffcfdff, // table_fill
  0xffd8dee7, // table_line
  0xff111827, // title_text
  0xff657287, // body_text
  0xff2c3848, // value_text
  0xff17233a, // badge_top
  0xff08111f, // badge_bottom
  0xffffffff, // badge_text
  0x2a081527, // badge_shadow
  0x5a1f2d46, // badge_ring
  0x16081527, // shadow_near
  0x050b1728, // shadow_far
  0x00000000, // accent_band
  2.0f, // card_border_width
  1.25f, // table_border_width
  1.05f, // table_divider_width
  -8.0f, // shadow_y_offset
  -2.0f, // shadow_clamp_offset
  15.0f, // shadow_near_height
  18.0f, // shadow_near_blur
  46.0f, // shadow_far_inset
  7.0f, // shadow_far_y_offset
  10.0f, // shadow_far_height
  26.0f, // shadow_far_blur
  0.0f, // accent_band_width
};

constexpr SampleTableCardStyle kSampleTableCardDarkBandStyle {
  0xff171b25, // card_face
  0xff2b3343, // card_border
  0xff111722, // table_fill
  0xff323b4c, // table_line
  0xfff5f7fb, // title_text
  0xffb8c2d1, // body_text
  0xffedf2f8, // value_text
  0xff2c3547, // badge_top
  0xff101722, // badge_bottom
  0xfff5f7fb, // badge_text
  0x26000000, // badge_shadow
  0x668b95a6, // badge_ring
  0x16081527, // shadow_near
  0x08081527, // shadow_far
  0xff8f36ff, // accent_band
  2.0f, // card_border_width
  1.2f, // table_border_width
  1.0f, // table_divider_width
  -10.0f, // shadow_y_offset
  -1.0f, // shadow_clamp_offset
  18.0f, // shadow_near_height
  24.0f, // shadow_near_blur
  44.0f, // shadow_far_inset
  7.0f, // shadow_far_y_offset
  12.0f, // shadow_far_height
  34.0f, // shadow_far_blur
  8.0f, // accent_band_width
};

constexpr SampleTableCardStyle makeSampleTableCardPaperInkStyle() {
  SampleTableCardStyle style = kSampleTableCardStyle;
  style.card_face = 0xfffffdf8;
  style.card_face_bottom = 0xfffbf7ee;
  style.card_border = 0xffded7c9;
  style.table_fill = 0xfffffcf5;
  style.table_fill_bottom = 0xfffbf6eb;
  style.table_line = 0xffd8d0c1;
  style.title_text = 0xff25231f;
  style.body_text = 0xff70675c;
  style.value_text = 0xff302d28;
  style.badge_top = 0xff25231f;
  style.badge_bottom = 0xff11100f;
  style.badge_text = 0xfffffdf8;
  style.badge_shadow = 0x20000000;
  style.badge_ring = 0x3025231f;
  style.shadow_near = 0x16000000;
  style.shadow_far = 0x06000000;
  style.card_border_width = 1.6f;
  style.table_border_width = 1.15f;
  style.table_divider_width = 1.0f;
  style.shadow_y_offset = -7.0f;
  style.shadow_clamp_offset = -1.0f;
  style.shadow_near_height = 13.0f;
  style.shadow_near_blur = 18.0f;
  style.shadow_far_inset = 48.0f;
  style.shadow_far_y_offset = 5.0f;
  style.shadow_far_height = 9.0f;
  style.shadow_far_blur = 24.0f;
  style.accent_line = 0xffd9545b;
  style.badge_shadow_offset = 4.0f;
  style.badge_shadow_blur = 12.0f;
  style.table_rounding_scale = 0.30f;
  style.card_surface = SampleCardSurface::VerticalGradient;
  style.table_rules = SampleTableRules::PaperDotted;
  style.badge_treatment = SampleBadgeTreatment::InkFilled;
  return style;
}

constexpr SampleTableCardStyle makeSampleTableCardSchematicStyle() {
  SampleTableCardStyle style = kSampleTableCardStyle;
  style.card_face = 0xffffffff;
  style.card_face_bottom = 0xfffefefe;
  style.card_border = 0xffe2e6ec;
  style.table_fill = 0xfffcfcfd;
  style.table_fill_bottom = 0xfffbfcfd;
  style.table_line = 0xffd9dde4;
  style.title_text = 0xff11151c;
  style.body_text = 0xff687486;
  style.value_text = 0xff25303d;
  style.badge_top = 0xffffffff;
  style.badge_bottom = 0xffffffff;
  style.badge_text = 0xff121821;
  style.badge_shadow = 0x00000000;
  style.badge_ring = 0xff172033;
  style.shadow_near = 0x11071a2e;
  style.shadow_far = 0x05071a2e;
  style.card_border_width = 1.4f;
  style.table_border_width = 1.25f;
  style.table_divider_width = 1.0f;
  style.shadow_y_offset = -6.0f;
  style.shadow_clamp_offset = -1.0f;
  style.shadow_near_height = 12.0f;
  style.shadow_near_blur = 16.0f;
  style.shadow_far_inset = 50.0f;
  style.shadow_far_y_offset = 5.0f;
  style.shadow_far_height = 8.0f;
  style.shadow_far_blur = 22.0f;
  style.accent_line = 0xffd94f55;
  style.badge_shadow_offset = 0.0f;
  style.badge_shadow_blur = 0.0f;
  style.table_rounding_scale = 0.26f;
  style.card_surface = SampleCardSurface::VerticalGradient;
  style.table_rules = SampleTableRules::SchematicLines;
  style.badge_treatment = SampleBadgeTreatment::LightOutline;
  return style;
}

constexpr SampleTableCardStyle makeSampleTableCardTechBandStyle() {
  SampleTableCardStyle style = kSampleTableCardStyle;
  style.card_face = 0xff202432;
  style.card_face_bottom = 0xff151925;
  style.card_border = 0xff2e374a;
  style.table_fill = 0xff111722;
  style.table_fill_bottom = 0xff0f141f;
  style.table_line = 0xff3a4357;
  style.title_text = 0xfff3f6fb;
  style.body_text = 0xffc2cbd8;
  style.value_text = 0xfff5f8fc;
  style.badge_top = 0x00000000;
  style.badge_bottom = 0x00000000;
  style.badge_text = 0xfff4f7fb;
  style.badge_shadow = 0x00000000;
  style.badge_ring = 0xffe4e9f2;
  style.shadow_near = 0x12000000;
  style.shadow_far = 0x05000000;
  style.accent_band = 0xff8d3cff;
  style.card_border_width = 1.5f;
  style.table_border_width = 1.1f;
  style.table_divider_width = 1.0f;
  style.shadow_y_offset = -6.0f;
  style.shadow_clamp_offset = -1.0f;
  style.shadow_near_height = 12.0f;
  style.shadow_near_blur = 18.0f;
  style.shadow_far_inset = 48.0f;
  style.shadow_far_y_offset = 5.0f;
  style.shadow_far_height = 8.0f;
  style.shadow_far_blur = 24.0f;
  style.accent_band_width = 8.0f;
  style.accent_line = 0xff8d3cff;
  style.detail_line = 0x6679859b;
  style.detail_dot = 0x5f9ba8bd;
  style.badge_shadow_offset = 0.0f;
  style.badge_shadow_blur = 0.0f;
  style.table_rounding_scale = 0.22f;
  style.card_surface = SampleCardSurface::VerticalGradient;
  style.table_rules = SampleTableRules::TechCells;
  style.badge_treatment = SampleBadgeTreatment::DarkOutline;
  return style;
}

constexpr SampleTableCardStyle kSampleTableCardPaperInkStyle = makeSampleTableCardPaperInkStyle();
constexpr SampleTableCardStyle kSampleTableCardSchematicStyle = makeSampleTableCardSchematicStyle();
constexpr SampleTableCardStyle kSampleTableCardTechBandStyle = makeSampleTableCardTechBandStyle();

SampleTableCardLayout sampleTableCardLayout(const Dimensions& dimensions) {
  const float width = static_cast<float>(dimensions.width);
  const float height = static_cast<float>(dimensions.height);
  const float short_side = std::min(width, height);
  const float margin = std::round(std::clamp(short_side * 0.105f, 18.0f, 30.0f));
  const float card_height =
      std::round(std::clamp(width * 0.112f, 136.0f, height - margin * 2.6f));
  const Rect card { margin,
                    std::round(std::max(margin, (height - card_height) * 0.5f - margin * 0.15f)),
                    width - 2.0f * margin,
                    card_height };
  const float radius = std::round(std::max(8.0f, height * 0.045f));
  const float badge_radius = std::clamp(card.height * 0.122f, 18.0f, 23.0f);
  const float badge_x = card.x + card.height * 0.31f;
  const float badge_y = card.y + card.height * 0.34f;
  const Rect copy { card.x + card.height * 0.54f,
                    card.y + card.height * 0.22f,
                    card.width * 0.18f,
                    card.height * 0.64f };
  const float table_x = std::round(card.x + std::max(card.width * 0.255f, 330.0f));
  const float table_right = std::round(card.x + card.width - card.height * 0.30f);
  const Rect table { table_x,
                     std::round(card.y + card.height * 0.24f),
                     table_right - table_x,
                     std::round(card.height * 0.52f) };

  return { card, table, copy, radius, badge_radius, badge_x, badge_y };
}

void drawCardShadow(visage::Canvas& canvas,
                    const SampleTableCardLayout& layout,
                    const SampleTableCardStyle& style) {
  const float shadow_x = layout.card.x + 14.0f;
  const float shadow_width = layout.card.width - 28.0f;
  const float shadow_y = layout.card.y + layout.card.height + style.shadow_y_offset;
  const float clamp_margin = 80.0f;
  const float clamp_top = layout.card.y + layout.card.height + style.shadow_clamp_offset;

  canvas.saveState();
  canvas.setClampBounds(layout.card.x - clamp_margin,
                        clamp_top,
                        layout.card.width + clamp_margin * 2.0f,
                        layout.card.height);

  if (hasAlpha(style.shadow_near)) {
    canvas.setColor(style.shadow_near);
    canvas.roundedRectangleShadow(
        shadow_x, shadow_y, shadow_width, style.shadow_near_height, layout.radius * 0.70f,
        style.shadow_near_blur);
  }

  if (hasAlpha(style.shadow_far)) {
    canvas.setColor(style.shadow_far);
    canvas.roundedRectangleShadow(shadow_x + style.shadow_far_inset,
                                  shadow_y + style.shadow_far_y_offset,
                                  shadow_width - style.shadow_far_inset * 2.0f,
                                  style.shadow_far_height,
                                  12.0f,
                                  style.shadow_far_blur);
  }

  canvas.restoreState();
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

void drawDottedVerticalRule(visage::Canvas& canvas,
                            float x,
                            float y,
                            float height,
                            float dot_size,
                            float gap,
                            uint32_t color) {
  canvas.setColor(color);
  for (float dot_y = y; dot_y <= y + height; dot_y += gap)
    canvas.fill(x - dot_size * 0.5f, dot_y - dot_size * 0.5f, dot_size, dot_size);
}

void drawDottedHorizontalRule(visage::Canvas& canvas,
                              float x,
                              float y,
                              float width,
                              float dot_size,
                              float gap,
                              uint32_t color) {
  canvas.setColor(color);
  for (float dot_x = x; dot_x <= x + width; dot_x += gap)
    canvas.fill(dot_x - dot_size * 0.5f, y - dot_size * 0.5f, dot_size, dot_size);
}

void drawSampleTableCardDetails(visage::Canvas& canvas,
                                const SampleTableCardLayout& layout,
                                const SampleTableCardStyle& style) {
  if (!hasAlpha(style.detail_line) && !hasAlpha(style.detail_dot))
    return;

  const float right = layout.card.x + layout.card.width - style.card_border_width - 22.0f;
  const float top = layout.card.y + style.card_border_width + 12.0f;
  const float bottom = layout.card.y + layout.card.height - style.card_border_width - 12.0f;

  if (hasAlpha(style.detail_line)) {
    const float corner_size = layout.card.height * 0.32f;
    visage::Path corner;
    corner.moveTo(right - corner_size * 0.58f, top);
    corner.lineTo(right - 14.0f, top);
    corner.lineTo(right, top + 14.0f);
    corner.lineTo(right, top + corner_size);
    fillStroke(canvas, corner, 1.0f, style.detail_line, {}, 0.0f, visage::Path::EndCap::Square);

    visage::Path lower;
    lower.moveTo(right - corner_size * 0.88f, bottom);
    lower.lineTo(right - corner_size * 0.18f, bottom);
    lower.lineTo(right, bottom - corner_size * 0.18f);
    lower.lineTo(right, bottom - corner_size * 0.78f);
    fillStroke(canvas, lower, 1.0f, style.detail_line, {}, 0.0f, visage::Path::EndCap::Square);
  }

  if (hasAlpha(style.detail_dot)) {
    const float dot = 1.45f;
    const float gap = 9.0f;
    const float origin_x = layout.card.x + layout.card.width - 168.0f;
    const float origin_y = layout.card.y + layout.card.height - 48.0f;
    canvas.setColor(style.detail_dot);
    for (int row = 0; row < 5; ++row) {
      for (int column = 0; column < 11; ++column) {
        const float fade = static_cast<float>(column + row) / 14.0f;
        if (fade < 0.15f)
          continue;
        canvas.fill(origin_x + column * gap, origin_y + row * gap, dot, dot);
      }
    }
  }
}

void drawSampleTableCardAccents(visage::Canvas& canvas,
                                const SampleTableCardLayout& layout,
                                const SampleTableCardStyle& style) {
  if (!hasAlpha(style.accent_line))
    return;

  if (style.table_rules == SampleTableRules::PaperDotted) {
    const float x = layout.table.x - 24.0f;
    const float y1 = layout.table.y + 10.0f;
    const float y2 = layout.table.y + layout.table.height - 10.0f;
    drawLine(canvas, x, y1, x, y2, 1.25f, alphaColor(180, style.accent_line), { 4.0f, 6.0f });
    canvas.setColor(style.accent_line);
    canvas.circle(x - 3.6f, (y1 + y2) * 0.5f - 3.6f, 7.2f);
  }
  else if (style.table_rules == SampleTableRules::SchematicLines) {
    const float x = layout.table.x - 20.0f;
    const float y1 = layout.table.y + layout.table.height * 0.20f;
    const float y2 = layout.table.y + layout.table.height * 0.80f;
    drawLine(canvas, x, y1, x, y2, 1.15f, alphaColor(150, style.accent_line), { 4.0f, 6.0f });
    canvas.setColor(style.accent_line);
    canvas.ring(x - 4.8f, (y1 + y2) * 0.5f - 4.8f, 9.6f, 1.55f);
  }
}

void drawSampleTableCardShell(visage::Canvas& canvas,
                              const SampleTableCardLayout& layout,
                              const SampleTableCardStyle& style) {
  drawCardShadow(canvas, layout, style);

  const float border_width = style.card_border_width;
  fillRoundedRectPath(canvas,
                      layout.card.x,
                      layout.card.y,
                      layout.card.width,
                      layout.card.height,
                      layout.radius,
                      style.card_border);

  if (style.card_surface == SampleCardSurface::VerticalGradient) {
    fillRoundedRectPath(canvas,
                        layout.card.x + border_width,
                        layout.card.y + border_width,
                        layout.card.width - border_width * 2.0f,
                        layout.card.height - border_width * 2.0f,
                        layout.radius - border_width,
                        visage::Brush::vertical(style.card_face, style.card_face_bottom));
  }
  else {
    fillRoundedRectPath(canvas,
                        layout.card.x + border_width,
                        layout.card.y + border_width,
                        layout.card.width - border_width * 2.0f,
                        layout.card.height - border_width * 2.0f,
                        layout.radius - border_width,
                        style.card_face);
  }

  if (style.accent_band_width > 0.0f) {
    canvas.setColor(style.accent_band);
    canvas.fill(layout.card.x + border_width,
                layout.card.y + border_width,
                style.accent_band_width,
                layout.card.height - border_width * 2.0f);
  }
}

void drawSampleTableCardBadge(visage::Canvas& canvas,
                              const SampleTableCardLayout& layout,
                              const SampleTableCardStyle& style) {
  const float diameter = layout.badge_radius * 2.0f;
  if (hasAlpha(style.badge_shadow) && style.badge_shadow_blur > 0.0f) {
    canvas.setColor(style.badge_shadow);
    canvas.roundedRectangleShadow(layout.badge_x - layout.badge_radius,
                                  layout.badge_y - layout.badge_radius + style.badge_shadow_offset,
                                  diameter,
                                  diameter,
                                  layout.badge_radius,
                                  style.badge_shadow_blur);
  }

  if (style.badge_treatment == SampleBadgeTreatment::LightOutline ||
      style.badge_treatment == SampleBadgeTreatment::DarkOutline) {
    if (hasAlpha(style.badge_top)) {
      canvas.setColor(style.badge_top);
      canvas.circle(layout.badge_x - layout.badge_radius, layout.badge_y - layout.badge_radius, diameter);
    }
    canvas.setColor(style.badge_ring);
    canvas.ring(layout.badge_x - layout.badge_radius, layout.badge_y - layout.badge_radius, diameter, 1.5f);
  }
  else {
    canvas.setColor(visage::Brush::vertical(style.badge_top, style.badge_bottom));
    canvas.circle(layout.badge_x - layout.badge_radius, layout.badge_y - layout.badge_radius, diameter);
    canvas.setColor(style.badge_ring);
    canvas.ring(layout.badge_x - layout.badge_radius, layout.badge_y - layout.badge_radius, diameter, 1.0f);
  }

  text(canvas,
       "1",
       layout.badge_radius * 0.82f,
       style.badge_text,
       visage::Font::kCenter,
       layout.badge_x - layout.badge_radius,
       layout.badge_y - layout.badge_radius - 1.0f,
       diameter,
       diameter);
}

void drawSampleTableCardCopy(visage::Canvas& canvas,
                             const SampleTableCardLayout& layout,
                             const SampleTableCardStyle& style) {
  const float title_size = std::clamp(layout.card.height * 0.145f, 20.0f, 25.0f);
  const float body_size = std::clamp(layout.card.height * 0.100f, 14.0f, 18.0f);
  const float title_line = title_size * 1.10f;
  const float body_y = layout.copy.y + title_line * 2.16f;

  fauxBoldText(canvas, "Stored as", title_size, style.title_text, visage::Font::kTopLeft,
               layout.copy.x, layout.copy.y, layout.copy.width, title_line);
  fauxBoldText(canvas, "sample values", title_size, style.title_text, visage::Font::kTopLeft,
               layout.copy.x, layout.copy.y + title_line, layout.copy.width, title_line);
  text(canvas, "Digital audio is stored", body_size, style.body_text, visage::Font::kTopLeft,
       layout.copy.x, body_y, layout.copy.width, body_size * 1.25f);
  text(canvas, "as a sequence of numbers.", body_size, style.body_text, visage::Font::kTopLeft,
       layout.copy.x, body_y + body_size * 1.18f, layout.copy.width, body_size * 1.25f);
}

void drawSampleValuesTable(visage::Canvas& canvas,
                           const SampleTableCardLayout& layout,
                           const SampleTableCardStyle& style) {
  constexpr std::array<std::string_view, 8> kValues {
    "0.00", "0.42", "0.88", "0.56", "-0.12", "-0.76", "-0.48", "0.22"
  };

  const float rounding = std::max(4.0f, layout.radius * style.table_rounding_scale);
  const float cell_width = layout.table.width / static_cast<float>(kValues.size());
  const float border_width = style.table_border_width;

  fillRoundedRectPath(canvas,
                      layout.table.x,
                      layout.table.y,
                      layout.table.width,
                      layout.table.height,
                      rounding,
                      style.table_line);

  if (style.card_surface == SampleCardSurface::VerticalGradient) {
    fillRoundedRectPath(canvas,
                        layout.table.x + border_width,
                        layout.table.y + border_width,
                        layout.table.width - border_width * 2.0f,
                        layout.table.height - border_width * 2.0f,
                        rounding - border_width,
                        visage::Brush::vertical(style.table_fill, style.table_fill_bottom));
  }
  else {
    fillRoundedRectPath(canvas,
                        layout.table.x + border_width,
                        layout.table.y + border_width,
                        layout.table.width - border_width * 2.0f,
                        layout.table.height - border_width * 2.0f,
                        rounding - border_width,
                        style.table_fill);
  }

  if (style.table_rules == SampleTableRules::PaperDotted) {
    const float top = layout.table.y + border_width + 6.0f;
    const float height = layout.table.height - border_width * 2.0f - 12.0f;
    for (size_t i = 1; i < kValues.size(); ++i) {
      const float x = layout.table.x + cell_width * static_cast<float>(i);
      drawDottedVerticalRule(canvas, x, top, height, 1.35f, 6.5f, style.table_line);
    }
    drawDottedHorizontalRule(canvas,
                             layout.table.x + 8.0f,
                             layout.table.y + layout.table.height * 0.50f,
                             layout.table.width - 16.0f,
                             1.15f,
                             7.0f,
                             alphaColor(80, style.table_line));
  }
  else {
    canvas.setColor(style.table_line);
    for (size_t i = 1; i < kValues.size(); ++i) {
      const float x = layout.table.x + cell_width * static_cast<float>(i);
      canvas.fill(x - style.table_divider_width * 0.5f,
                  layout.table.y + border_width,
                  style.table_divider_width,
                  layout.table.height - border_width * 2.0f);
    }

    if (style.table_rules == SampleTableRules::TechCells && hasAlpha(style.accent_line)) {
      canvas.setColor(alphaColor(128, style.accent_line));
      canvas.fill(layout.table.x + border_width,
                  layout.table.y + border_width,
                  layout.table.width - border_width * 2.0f,
                  1.0f);
    }
  }

  const float value_size = std::clamp(layout.table.height * 0.28f, 18.0f, 22.0f);
  for (size_t i = 0; i < kValues.size(); ++i) {
    text(canvas,
         std::string(kValues[i]),
         value_size,
         style.value_text,
         visage::Font::kCenter,
         layout.table.x + cell_width * static_cast<float>(i),
         layout.table.y,
         cell_width,
         layout.table.height);
  }
}

void drawSampleTableCard(DrawContext& context,
                         const Dimensions& dimensions,
                         const SampleTableCardStyle& style = kSampleTableCardStyle) {
  SampleTableCardLayout layout = sampleTableCardLayout(dimensions);
  drawSampleTableCardShell(context.canvas, layout, style);
  drawSampleTableCardDetails(context.canvas, layout, style);
  drawSampleTableCardAccents(context.canvas, layout, style);
  drawSampleTableCardBadge(context.canvas, layout, style);
  drawSampleTableCardCopy(context.canvas, layout, style);
  drawSampleValuesTable(context.canvas, layout, style);
}

void drawDot(visage::Canvas& canvas, float x, float y, float radius, uint32_t color) {
  canvas.setColor(color);
  canvas.circle(x - radius, y - radius, radius * 2.0f);
}

void drawOpenDot(visage::Canvas& canvas, float x, float y, float radius, uint32_t color, float width) {
  canvas.setColor(color);
  canvas.ring(x - radius, y - radius, radius * 2.0f, width);
}

std::vector<SignalPoint> makeSingleCycle(const Rect& area,
                                         int samples,
                                         float phase_cycles = 0.0f,
                                         float vertical_scale = 0.42f) {
  std::vector<SignalPoint> points;
  points.reserve(static_cast<size_t>(samples));
  const float center = area.y + area.height * 0.5f;
  const float amplitude = area.height * vertical_scale;

  for (int i = 0; i < samples; ++i) {
    const float t = static_cast<float>(i) / static_cast<float>(samples - 1);
    const float value = std::sin(2.0f * kPi * (t + phase_cycles));
    points.push_back({ t, area.x + t * area.width, center - value * amplitude, value });
  }

  return points;
}

SignalPoint pointAt(const std::vector<SignalPoint>& points, float t) {
  if (points.empty())
    return {};

  const size_t index = static_cast<size_t>(
    std::clamp(t, 0.0f, 1.0f) * static_cast<float>(points.size() - 1));
  return points[index];
}

void drawBlueArea(visage::Canvas& canvas, const Dimensions& dimensions) {
  drawBackground(canvas, dimensions, 0xff1d242d, 0xff111820, 0x20395f95, 0.42f, 0.22f);
  const Rect area = studyArea(dimensions, 0.0f, 0.0f, 0.0f, 34.0f);
  drawGrid(canvas, area, 18, 9, 3, 0x172e3a46, 0x2b43505f);

  const float axis_y = area.y + area.height * 0.5f;
  drawLine(canvas, area.x, axis_y, area.x + area.width, axis_y, 1.4f, 0x70d6dde8);

  const auto sine = makeSingleCycle(area, 1100, 0.0f, 0.42f);
  const visage::Path wave = pathFromPoints(sine);
  canvas.setColor(visage::Brush::vertical(0x3f86a6df, 0x071f2d43));
  for (const visage::Path& lobe : fillLobesToBaseline(sine, axis_y))
    canvas.fill(lobe);

  fillStroke(canvas, wave, 12.0f, 0x167aa6e8);
  fillStroke(canvas, wave, 5.0f, 0x2f86b5ff);
  fillStroke(canvas, wave, 2.0f, 0xff9fc4ff);
  fillStroke(canvas, wave, 0.75f, 0x95eef6ff);
}

void drawCreamMarkers(DrawContext& context, const Dimensions& dimensions) {
  visage::Canvas& canvas = context.canvas;
  drawBackground(canvas, dimensions, 0xff111210, 0xff0a0b09, 0x151d1b15, 0.18f, 0.38f);
  const Rect area = studyArea(dimensions, 0.0f, 24.0f, 0.0f, 30.0f);
  const float axis_y = area.y + area.height * 0.5f;

  drawLine(canvas, area.x, axis_y, area.x + area.width, axis_y, 1.2f, 0x90b4b1aa);
  for (float guide : { 0.0f, 0.37f, 0.68f, 1.0f }) {
    const float x = area.x + area.width * guide;
    drawLine(canvas, x, area.y, x, area.y + area.height, 1.4f, 0x7b8d6f62, { 2.0f, 8.0f });
  }

  const auto sine = makeSingleCycle(area, 1100, 0.0f, 0.44f);
  const visage::Path wave = pathFromPoints(sine);
  canvas.setColor(visage::Brush::vertical(0x17d9d1bf, 0x00000000));
  for (const visage::Path& lobe : fillLobesToBaseline(sine, axis_y))
    canvas.fill(lobe);

  visage::Region& shadow = addBlurRegion(context, 14.0f);
  drawInRegion(context, shadow, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, pathFromPoints(sine, 4.5f, 7.0f), 8.0f, 0x68000000);
  });

  visage::Region& foreground = addRegion(context, true);
  drawInRegion(context, foreground, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, wave, 3.1f, 0xffeee8d9);

    for (float marker : { 0.25f, 0.50f, 0.75f }) {
      const SignalPoint point = pointAt(sine, marker);
      drawDot(region_canvas, point.x, point.y, 5.5f, 0xffdf8fa5);
    }

    const SignalPoint end = pointAt(sine, 1.0f);
    drawDot(region_canvas, end.x, end.y, 5.0f, 0xfff0eadf);
  });
}

void drawPaperInk(DrawContext& context, const Dimensions& dimensions) {
  visage::Canvas& canvas = context.canvas;
  canvas.setColor(visage::Brush::vertical(0xfffaf7ef, 0xfff1ede3));
  canvas.fill(0, 0, dimensions.width, dimensions.height);

  const Rect area = studyArea(dimensions, 58.0f, 30.0f, 58.0f, 30.0f);
  drawGrid(canvas, area, 12, 6, 6, 0x1e9c9a8e, 0x4a5f6266);

  const float axis_y = area.y + area.height * 0.5f;
  drawLine(canvas, area.x, axis_y, area.x + area.width, axis_y, 1.6f, 0xff8a8d90, { 2.0f, 6.0f });
  drawLine(canvas, area.x + area.width * 0.50f, area.y, area.x + area.width * 0.50f,
           area.y + area.height, 1.5f, 0xff5f6266, { 2.0f, 6.0f });

  const auto primary = makeSingleCycle(area, 1000, 0.02f, 0.35f);
  const auto secondary = makeSingleCycle(area, 1000, -0.16f, 0.30f);
  const visage::Path primary_path = pathFromPoints(primary);
  const visage::Path secondary_path = pathFromPoints(secondary);

  visage::Region& shadow = addBlurRegion(context, 22.0f);
  drawInRegion(context, shadow, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, pathFromPoints(primary, 7.0f, 11.0f), 14.0f, 0x32000000);
  });

  visage::Region& foreground = addRegion(context, true);
  drawInRegion(context, foreground, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, primary_path, 2.6f, 0xff252525);
    fillStroke(region_canvas, secondary_path, 2.1f, 0xffdb5b68, { 9.0f, 9.0f });

    const SignalPoint start = pointAt(primary, 0.0f);
    const SignalPoint red_a = pointAt(secondary, 0.25f);
    const SignalPoint black_open = pointAt(primary, 0.66f);
    const SignalPoint red_end = pointAt(secondary, 1.0f);
    drawDot(region_canvas, start.x, start.y, 10.0f, 0xff252525);
    drawOpenDot(region_canvas, red_a.x, red_a.y, 10.0f, 0xffd84d5d, 3.0f);
    drawOpenDot(region_canvas, black_open.x, black_open.y, 10.0f, 0xff252525, 3.0f);
    drawDot(region_canvas, red_end.x, red_end.y, 9.0f, 0xffdf4655);
  });
}

void drawMonoChirp(DrawContext& context, const Dimensions& dimensions) {
  visage::Canvas& canvas = context.canvas;
  drawBackground(canvas, dimensions, 0xff1a1c1b, 0xff0d0e0d, 0x141f1f1f, 0.20f, 0.40f);
  const Rect area = studyArea(dimensions, 0.0f, 24.0f, 0.0f, 28.0f);
  drawGrid(canvas, area, 28, 4, 4, 0x26434645, 0x5d5c6260);

  const float axis_y = area.y + area.height * 0.52f;
  drawLine(canvas, area.x, axis_y, area.x + area.width, axis_y, 1.7f, 0xbfcac7bd);

  const auto chirp = makeChirp(area, 1600, 0.12f);
  const visage::Path wave = pathFromPoints(chirp);
  canvas.setColor(visage::Brush::vertical(0x20efeee6, 0x00000000));
  for (const visage::Path& lobe : fillLobesToBaseline(chirp, axis_y))
    canvas.fill(lobe);

  visage::Region& shadow = addBlurRegion(context, 20.0f);
  drawInRegion(context, shadow, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, pathFromPoints(chirp, 0.0f, 8.0f), 10.0f, 0x52000000);
  });

  visage::Region& foreground = addRegion(context, true);
  drawInRegion(context, foreground, [&](visage::Canvas& region_canvas) {
    fillStroke(region_canvas, wave, 3.0f, 0xffebeae1);
  });
}

} // namespace

const std::array<StyleStudy, 19>& styleStudies() {
  return kStyleStudies;
}

std::optional<StyleStudy> styleStudyById(std::string_view id) {
  const auto& studies = styleStudies();
  const auto it = std::find_if(studies.begin(), studies.end(), [id](const StyleStudy& study) {
    return study.id == id;
  });

  if (it == studies.end())
    return std::nullopt;

  return *it;
}

void drawStyleStudy(DrawContext& context,
                    std::string_view study_id,
                    const Dimensions& dimensions,
                    const Timeline& timeline) {
  (void)timeline;
  visage::Canvas& canvas = context.canvas;

  if (study_id == "warm-scope")
    drawWarmScope(canvas, dimensions);
  else if (study_id == "spectral-callout")
    drawSpectralCallout(canvas, dimensions);
  else if (study_id == "blue-decay")
    drawBlueDecay(canvas, dimensions);
  else if (study_id == "quiet-overlay")
    drawQuietOverlay(canvas, dimensions);
  else if (study_id == "filled-ribbon")
    drawFilledRibbon(canvas, dimensions);
  else if (study_id == "blue-area")
    drawBlueArea(canvas, dimensions);
  else if (study_id == "cream-markers")
    drawCreamMarkers(context, dimensions);
  else if (study_id == "paper-ink")
    drawPaperInk(context, dimensions);
  else if (study_id == "mono-chirp")
    drawMonoChirp(context, dimensions);
  else if (study_id == "blue-ridge")
    drawBlueRidge(context, dimensions);
  else if (study_id == "blue-ridge-framed")
    drawBlueRidgeFramed(context, dimensions);
  else if (study_id == "sample-table-card")
    drawSampleTableCard(context, dimensions);
  else if (study_id == "sample-table-card-page")
    drawSampleTableCard(context, dimensions, kSampleTableCardPageStyle);
  else if (study_id == "sample-table-card-paper")
    drawSampleTableCard(context, dimensions, kSampleTableCardPaperStyle);
  else if (study_id == "sample-table-card-tight")
    drawSampleTableCard(context, dimensions, kSampleTableCardTightStyle);
  else if (study_id == "sample-table-card-dark-band")
    drawSampleTableCard(context, dimensions, kSampleTableCardDarkBandStyle);
  else if (study_id == "sample-table-card-paper-ink")
    drawSampleTableCard(context, dimensions, kSampleTableCardPaperInkStyle);
  else if (study_id == "sample-table-card-schematic")
    drawSampleTableCard(context, dimensions, kSampleTableCardSchematicStyle);
  else if (study_id == "sample-table-card-tech-band")
    drawSampleTableCard(context, dimensions, kSampleTableCardTechBandStyle);
  else
    throw std::runtime_error("Unknown style study: " + std::string(study_id));
}

visage::Screenshot renderStyleStudyFrame(std::string_view study_id,
                                         const Dimensions& dimensions,
                                         const Timeline& timeline) {
  visage::Canvas canvas;
  canvas.setWindowless(dimensions.width, dimensions.height);
  canvas.updateTime(timeline.time_seconds);
  DrawContext context(canvas, dimensions);
  drawStyleStudy(context, study_id, dimensions, timeline);
  canvas.submit();
  return canvas.takeScreenshot();
}

void saveStyleStudyFrame(const std::string& output_path,
                         std::string_view study_id,
                         const Dimensions& dimensions,
                         const Timeline& timeline) {
  savePngWithStraightAlpha(output_path, renderStyleStudyFrame(study_id, dimensions, timeline));
}

} // namespace adt
