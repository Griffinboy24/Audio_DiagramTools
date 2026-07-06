#include "audio_diagram_tools/canonical_renderers.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace adt::canonical::renderers {
namespace {

using drawing::DrawContext;
using drawing::Rect;

constexpr float kPi = drawing::kPi;

constexpr uint32_t kBackground = 0xff1d1d1d;
constexpr uint32_t kBlockFillTop = 0xff3a453f;
constexpr uint32_t kBlockFillBottom = 0xff29312d;
constexpr uint32_t kBlockOuterStroke = 0xff0c0c0c;
constexpr uint32_t kBlockBevel = 0xff78c991;
constexpr uint32_t kWave = 0xff8ed69f;
constexpr uint32_t kWaveShadow = 0xff0f130f;
constexpr uint32_t kMidline = 0xff474747;
constexpr uint32_t kReadHead = 0xffbaf5c7;
constexpr uint32_t kReadHeadDim = 0xff5f8a6a;

uint8_t channel(uint32_t color, int shift) {
  return static_cast<uint8_t>((color >> shift) & 0xffu);
}

uint32_t scaleAlpha(uint32_t color, float alpha_scale) {
  const uint8_t alpha = static_cast<uint8_t>(std::lround(
      static_cast<float>(channel(color, 24)) * drawing::clamp01(alpha_scale)));
  return (static_cast<uint32_t>(alpha) << 24) | (color & 0x00ffffffu);
}

float clamp01(float value) {
  return drawing::clamp01(value);
}

float easeOutCubic(float value) {
  value = clamp01(value);
  const float inverse = 1.0f - value;
  return 1.0f - inverse * inverse * inverse;
}

float outputValue(float local_cycles) {
  const float sine = std::sin(2.0f * kPi * local_cycles);
  const float saturated = std::tanh(sine * 2.35f) / std::tanh(2.35f);
  return saturated * 0.58f * 0.84f;
}

struct WavePoint {
  float x = 0.0f;
  float y = 0.0f;
  float value = 0.0f;
};

std::vector<WavePoint> makeWavePoints(const Rect& plot, float block_index) {
  constexpr int kSamples = 80;
  std::vector<WavePoint> points;
  points.reserve(kSamples);

  const float center_y = plot.y + plot.height * 0.5f;
  const float amplitude = plot.height * 0.46f;

  for (int i = 0; i < kSamples; ++i) {
    const float local = static_cast<float>(i) / static_cast<float>(kSamples - 1);
    const float value = outputValue(block_index + local);
    points.push_back({
        plot.x + plot.width * local,
        center_y - value * amplitude,
        value,
    });
  }

  return points;
}

visage::Path wavePath(const std::vector<WavePoint>& points) {
  visage::Path path;
  for (size_t i = 0; i < points.size(); ++i) {
    if (i == 0)
      path.moveTo(points[i].x, points[i].y);
    else
      path.lineTo(points[i].x, points[i].y);
  }
  return path;
}

std::vector<visage::Path> waveFillPaths(const std::vector<WavePoint>& points, float center_y) {
  std::vector<visage::Path> fills;
  if (points.size() < 2)
    return fills;

  std::vector<WavePoint> lobe;
  lobe.push_back(points.front());

  const auto same_side = [](float a, float b) {
    return (a >= 0.0f && b >= 0.0f) || (a <= 0.0f && b <= 0.0f);
  };

  const auto add_lobe = [&](const std::vector<WavePoint>& source) {
    if (source.size() < 2)
      return;

    visage::Path fill;
    fill.moveTo(source.front().x, center_y);
    for (const WavePoint& point : source)
      fill.lineTo(point.x, point.y);
    fill.lineTo(source.back().x, center_y);
    fill.close();
    fills.push_back(fill);
  };

  for (size_t i = 1; i < points.size(); ++i) {
    const WavePoint& previous = points[i - 1];
    const WavePoint& point = points[i];
    if (!same_side(previous.value, point.value)) {
      const float denom = previous.value - point.value;
      const float t = std::abs(denom) < 0.00001f ? 0.0f : previous.value / denom;
      const WavePoint crossing {
        previous.x + (point.x - previous.x) * t,
        center_y,
        0.0f,
      };
      lobe.push_back(crossing);
      add_lobe(lobe);
      lobe.clear();
      lobe.push_back(crossing);
    }

    lobe.push_back(point);
  }

  add_lobe(lobe);
  return fills;
}

void drawOutputBlock(visage::Canvas& canvas,
                     const Rect& block,
                     float block_index,
                     float opacity) {
  opacity = clamp01(opacity);
  if (opacity <= 0.0f)
    return;

  constexpr float kRadius = 2.75f;
  canvas.setColor(scaleAlpha(0xff000000, opacity * 0.22f));
  canvas.roundedRectangle(block.x + 2.0f, block.y + 2.0f, block.width, block.height, kRadius);

  canvas.setColor(scaleAlpha(kBlockOuterStroke, opacity));
  canvas.roundedRectangle(block.x, block.y, block.width, block.height, kRadius);
  canvas.setColor(visage::Brush::vertical(scaleAlpha(kBlockFillTop, opacity),
                                          scaleAlpha(kBlockFillBottom, opacity)));
  canvas.roundedRectangle(block.x + 1.0f,
                          block.y + 1.0f,
                          block.width - 2.0f,
                          block.height - 2.0f,
                          kRadius - 0.75f);
  canvas.setColor(scaleAlpha(kBlockBevel, opacity));
  canvas.roundedRectangleBorder(block.x + 0.5f,
                                block.y + 0.5f,
                                block.width - 1.0f,
                                block.height - 1.0f,
                                kRadius,
                                1.0f);
  canvas.setColor(scaleAlpha(0xff0b0b0b, opacity * 0.68f));
  canvas.roundedRectangleBorder(block.x + 3.0f,
                                block.y + 3.0f,
                                block.width - 6.0f,
                                block.height - 6.0f,
                                kRadius - 1.2f,
                                0.75f);

  const Rect plot { block.x + 2.0f, block.y + 7.0f, block.width - 4.0f, block.height - 14.0f };
  const float center_y = block.y + block.height * 0.5f;
  canvas.setColor(scaleAlpha(0xff161616, opacity * 0.38f));
  canvas.roundedRectangle(plot.x, plot.y, plot.width, plot.height, 1.4f);

  for (int i = 1; i < 4; ++i) {
    const float tick_x = plot.x + plot.width * static_cast<float>(i) / 4.0f;
    canvas.setColor(scaleAlpha(0xff3a3a3a, opacity * 0.72f));
    canvas.fill(tick_x - 0.45f, plot.y + 3.0f, 0.9f, plot.height - 6.0f);
  }

  drawing::drawLine(canvas,
                    plot.x + 1.0f,
                    center_y,
                    plot.x + plot.width - 1.0f,
                    center_y,
                    0.65f,
                    scaleAlpha(kMidline, opacity * 0.82f));

  const std::vector<WavePoint> points = makeWavePoints(plot, block_index);
  canvas.setColor(scaleAlpha(kWave, opacity * 0.12f));
  for (const visage::Path& fill_path : waveFillPaths(points, center_y))
    canvas.fill(fill_path);

  const visage::Path wave = wavePath(points);
  drawing::fillStroke(canvas, wave, 2.8f, scaleAlpha(kWaveShadow, opacity * 0.78f));
  drawing::fillStroke(canvas, wave, 1.3f, scaleAlpha(kWave, opacity));
}

void drawReadHead(visage::Canvas& canvas, float x, float y, float height) {
  drawing::drawLine(canvas, x, y, x, y + height, 5.2f, scaleAlpha(kReadHeadDim, 0.22f));
  drawing::drawLine(canvas, x, y + 4.0f, x, y + height - 4.0f, 1.65f, kReadHead);
  canvas.setColor(scaleAlpha(kReadHead, 0.18f));
  canvas.roundedRectangle(x - 5.0f, y - 8.0f, 10.0f, height + 16.0f, 5.0f);
}

float sampleAtReadHead(float read_head_x,
                       float stream_left_x,
                       float block_width,
                       float phase_blocks) {
  const float audio_coordinate = (read_head_x - stream_left_x) / block_width - phase_blocks;
  return outputValue(audio_coordinate);
}

void drawOutputStreamToSpeaker(DrawContext& context,
                               const Dimensions& dimensions,
                               const Timeline& timeline,
                               const OutputStreamToSpeakerOptions& options) {
  visage::Canvas& canvas = context.canvas;
  const float w = static_cast<float>(dimensions.width);
  const float h = static_cast<float>(dimensions.height);

  if (options.clear_background) {
    canvas.setColor(kBackground);
    canvas.fill(0, 0, dimensions.width, dimensions.height);
  }

  constexpr float block_width = 76.0f;
  constexpr float block_height = 40.0f;
  const float read_head_x = 592.0f;
  const float chain_start_x = block_width * 3.0f;
  const float stream_center_y = h * 0.50f;
  const float stream_top = stream_center_y - block_height * 0.5f;
  const float phase_blocks =
      static_cast<float>(timeline.normalized_time - std::floor(timeline.normalized_time));

  constexpr float kFlyStart = 0.035f;
  constexpr float kFlyEnd = 0.22f;
  const float attached_incoming_x = chain_start_x + (phase_blocks - 1.0f) * block_width;
  if (phase_blocks >= kFlyStart) {
    const float fly_t = easeOutCubic((phase_blocks - kFlyStart) / (kFlyEnd - kFlyStart));
    const float incoming_start_x = -block_width * 1.12f;
    const float incoming_x = phase_blocks < kFlyEnd
                                 ? incoming_start_x +
                                       (attached_incoming_x - incoming_start_x) * fly_t
                                 : attached_incoming_x;
    if (incoming_x + block_width > -6.0f && incoming_x < read_head_x + block_width) {
      drawOutputBlock(canvas,
                      { incoming_x, stream_top, block_width, block_height },
                      -1.0f,
                      0.94f);
    }
  }

  for (int slot = 0; slot <= 7; ++slot) {
    const float x = chain_start_x + (static_cast<float>(slot) + phase_blocks) * block_width;
    if (x + block_width < -6.0f || x > read_head_x + block_width)
      continue;

    drawOutputBlock(canvas,
                    { x, stream_top, block_width, block_height },
                    static_cast<float>(slot),
                    0.94f);
  }

  canvas.setColor(kBackground);
  canvas.rectangle(read_head_x + 1.0f,
                   stream_top - 18.0f,
                   w - read_head_x - 1.0f,
                   block_height + 36.0f);

  const float drive = sampleAtReadHead(read_head_x, chain_start_x, block_width, phase_blocks);
  drawSpeakerConeMotionExperimentAt(context,
                                    dimensions,
                                    timeline,
                                    w - 376.0f,
                                    66.0f,
                                    0.37f,
                                    false,
                                    false,
                                    drive,
                                    drive);

  visage::Region& read_head_region = drawing::addRegion(context, true);
  drawing::drawInRegion(context, read_head_region, [&](visage::Canvas& read_head_canvas) {
    drawReadHead(read_head_canvas, read_head_x, stream_top - 10.0f, block_height + 20.0f);
  });
}

} // namespace

void drawOutputStreamToSpeakerGraphic(DrawContext& context,
                                      const Dimensions& dimensions,
                                      const Timeline& timeline,
                                      const OutputStreamToSpeakerOptions& options) {
  drawOutputStreamToSpeaker(context, dimensions, timeline, options);
}

} // namespace adt::canonical::renderers
