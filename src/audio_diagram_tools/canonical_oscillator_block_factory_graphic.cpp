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
constexpr uint32_t kBlockFillTop = 0xff333333;
constexpr uint32_t kBlockFillBottom = 0xff242424;
constexpr uint32_t kGeneratedFillTop = 0xff2d3a33;
constexpr uint32_t kGeneratedFillBottom = 0xff202822;
constexpr uint32_t kActiveFillTop = 0xff3b4640;
constexpr uint32_t kActiveFillBottom = 0xff29312d;
constexpr uint32_t kBlockOuterStroke = 0xff0c0c0c;
constexpr uint32_t kBlockBevel = 0xff5c5c5c;
constexpr uint32_t kGeneratedBevel = 0xff607265;
constexpr uint32_t kActiveBevel = 0xff7acd93;
constexpr uint32_t kOutputWave = 0xff86c799;
constexpr uint32_t kWaveShadow = 0xff101410;
constexpr uint32_t kBlockMidline = 0xff474747;
constexpr uint32_t kBlockTick = 0xff3a3a3a;
constexpr uint32_t kActiveGlow = 0xff7acd93;

constexpr float kWavePeriodBlocks = 5.0f;
constexpr float kSineCyclesPerPeriod = 2.0f;
constexpr float kWavePhaseOffsetCycles = 0.137f;
constexpr float kLoopAge = 6.0f;
constexpr float kLoopBlockCycles = kWavePeriodBlocks;
constexpr float kLoopStartOffsetBlocks = 0.43f;
constexpr float kApproachEndAge = 1.45f;
constexpr float kHoldBeforeLiftEndAge = 1.62f;
constexpr float kLiftEndAge = 2.16f;
constexpr float kProcessEndAge = 3.55f;
constexpr float kDropEndAge = 4.15f;

uint8_t channel(uint32_t color, int shift) {
  return static_cast<uint8_t>((color >> shift) & 0xffu);
}

uint32_t scaleAlpha(uint32_t color, float alpha_scale) {
  const uint8_t alpha = static_cast<uint8_t>(std::lround(
      static_cast<float>(channel(color, 24)) * drawing::clamp01(alpha_scale)));
  return (static_cast<uint32_t>(alpha) << 24) | (color & 0x00ffffffu);
}

uint32_t lerpColor(uint32_t a, uint32_t b, float t) {
  t = drawing::clamp01(t);
  const auto lerp_channel = [t](uint8_t from, uint8_t to) {
    return static_cast<uint8_t>(std::lround(static_cast<float>(from) +
                                           (static_cast<float>(to) - static_cast<float>(from)) * t));
  };

  return (static_cast<uint32_t>(lerp_channel(channel(a, 24), channel(b, 24))) << 24) |
         (static_cast<uint32_t>(lerp_channel(channel(a, 16), channel(b, 16))) << 16) |
         (static_cast<uint32_t>(lerp_channel(channel(a, 8), channel(b, 8))) << 8) |
         static_cast<uint32_t>(lerp_channel(channel(a, 0), channel(b, 0)));
}

float clamp01(float value) {
  return drawing::clamp01(value);
}

float ease(float value) {
  return drawing::smoothstep(0.0f, 1.0f, clamp01(value));
}

float interval(float value, float start, float end) {
  return clamp01((value - start) / (end - start));
}

float lerp(float a, float b, float t) {
  return a + (b - a) * t;
}

float wrapBlockCoordinate(float x_blocks) {
  float wrapped = std::fmod(x_blocks, kWavePeriodBlocks);
  if (wrapped < 0.0f)
    wrapped += kWavePeriodBlocks;
  return wrapped;
}

float sineOutput(float x_blocks) {
  const float x = wrapBlockCoordinate(x_blocks) / kWavePeriodBlocks;
  const float sine =
      std::sin(2.0f * kPi * (x * kSineCyclesPerPeriod + kWavePhaseOffsetCycles));
  return std::clamp(sine * 0.84f, -0.92f, 0.92f);
}

struct BlockWavePoint {
  float x = 0.0f;
  float y = 0.0f;
  float value = 0.0f;
};

std::vector<BlockWavePoint> sineWavePoints(const Rect& block,
                                           float audio_start_blocks,
                                           float draw_progress) {
  draw_progress = clamp01(draw_progress);
  std::vector<BlockWavePoint> points;
  if (draw_progress <= 0.001f)
    return points;

  const Rect plot { block.x + 1.0f, block.y + 7.0f, block.width - 2.0f, block.height - 14.0f };
  const float center_y = plot.y + plot.height * 0.5f;
  const float amplitude = plot.height * 0.46f;
  constexpr int kSamples = 72;
  const float start_local = 1.0f - draw_progress;
  points.reserve(kSamples);

  for (int i = 0; i < kSamples; ++i) {
    const float t = static_cast<float>(i) / static_cast<float>(kSamples - 1);
    const float local = lerp(start_local, 1.0f, t);
    const float output_stream_local = 1.0f - local;
    const float value = sineOutput(audio_start_blocks + output_stream_local);
    points.push_back({
        plot.x + plot.width * local,
        center_y - value * amplitude,
        value,
    });
  }

  return points;
}

visage::Path wavePath(const std::vector<BlockWavePoint>& points) {
  visage::Path path;
  for (size_t i = 0; i < points.size(); ++i) {
    if (i == 0)
      path.moveTo(points[i].x, points[i].y);
    else
      path.lineTo(points[i].x, points[i].y);
  }
  return path;
}

std::vector<visage::Path> waveFillPaths(const std::vector<BlockWavePoint>& points,
                                        float center_y) {
  std::vector<visage::Path> fills;
  if (points.size() < 2)
    return fills;

  std::vector<BlockWavePoint> lobe;
  lobe.push_back(points.front());

  const auto same_side = [](float a, float b) {
    return (a >= 0.0f && b >= 0.0f) || (a <= 0.0f && b <= 0.0f);
  };

  const auto add_lobe = [&](const std::vector<BlockWavePoint>& source) {
    if (source.size() < 2)
      return;

    visage::Path fill;
    fill.moveTo(source.front().x, center_y);
    for (const BlockWavePoint& point : source)
      fill.lineTo(point.x, point.y);
    fill.lineTo(source.back().x, center_y);
    fill.close();
    fills.push_back(fill);
  };

  for (size_t i = 1; i < points.size(); ++i) {
    const BlockWavePoint& previous = points[i - 1];
    const BlockWavePoint& point = points[i];

    if (!same_side(previous.value, point.value)) {
      const float denom = previous.value - point.value;
      const float t = std::abs(denom) < 0.00001f ? 0.0f : previous.value / denom;
      const BlockWavePoint crossing { lerp(previous.x, point.x, t), center_y, 0.0f };
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

void drawBlock(visage::Canvas& canvas,
               const Rect& block,
               float audio_start_blocks,
               float generated_amount,
               float draw_progress,
               float active_amount,
               float opacity = 1.0f) {
  opacity = clamp01(opacity);
  if (opacity <= 0.0f)
    return;

  generated_amount = clamp01(generated_amount);
  draw_progress = clamp01(draw_progress);
  const float outline_amount = ease(generated_amount);
  const uint32_t fill_top = scaleAlpha(
      lerpColor(lerpColor(kBlockFillTop, kGeneratedFillTop, generated_amount),
                kActiveFillTop,
                active_amount * outline_amount),
      opacity);
  const uint32_t fill_bottom = scaleAlpha(
      lerpColor(lerpColor(kBlockFillBottom, kGeneratedFillBottom, generated_amount),
                kActiveFillBottom,
                active_amount * outline_amount),
      opacity);
  const uint32_t bevel =
      scaleAlpha(lerpColor(kBlockBevel, kActiveBevel, outline_amount), opacity);
  constexpr float kRadius = 2.75f;

  if (outline_amount > 0.03f) {
    canvas.setColor(scaleAlpha(kActiveGlow, outline_amount * opacity * 0.13f));
    canvas.roundedRectangle(block.x - 3.0f,
                            block.y - 3.0f,
                            block.width + 6.0f,
                            block.height + 6.0f,
                            kRadius + 2.0f);
  }

  canvas.setColor(scaleAlpha(0xff000000, opacity * 0.22f));
  canvas.roundedRectangle(block.x + 2.0f, block.y + 2.0f, block.width, block.height, kRadius);

  canvas.setColor(scaleAlpha(kBlockOuterStroke, opacity));
  canvas.roundedRectangle(block.x, block.y, block.width, block.height, kRadius);
  canvas.setColor(visage::Brush::vertical(fill_top, fill_bottom));
  canvas.roundedRectangle(block.x + 1.0f,
                          block.y + 1.0f,
                          block.width - 2.0f,
                          block.height - 2.0f,
                          kRadius - 0.75f);
  canvas.setColor(bevel);
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

  const float center_y = block.y + block.height * 0.5f;
  const Rect plot { block.x + 8.0f, block.y + 8.0f, block.width - 16.0f, block.height - 16.0f };
  canvas.setColor(scaleAlpha(0xff161616, opacity * 0.38f));
  canvas.roundedRectangle(plot.x, plot.y, plot.width, plot.height, 1.4f);

  for (int i = 1; i < 4; ++i) {
    const float tick_x = plot.x + plot.width * static_cast<float>(i) / 4.0f;
    canvas.setColor(scaleAlpha(kBlockTick, opacity * 0.72f));
    canvas.fill(tick_x - 0.45f, plot.y + 3.0f, 0.9f, plot.height - 6.0f);
  }

  drawing::drawLine(canvas,
                    plot.x + 1.0f,
                    center_y,
                    plot.x + plot.width - 1.0f,
                    center_y,
                    0.65f,
                    scaleAlpha(kBlockMidline, opacity * 0.82f));

  const std::vector<BlockWavePoint> wave_points =
      sineWavePoints(block, audio_start_blocks, draw_progress);
  if (wave_points.size() >= 2) {
    canvas.setColor(scaleAlpha(kOutputWave, opacity * 0.12f));
    for (const visage::Path& fill_path : waveFillPaths(wave_points, center_y))
      canvas.fill(fill_path);

    const visage::Path wave = wavePath(wave_points);
    drawing::fillStroke(canvas, wave, 2.9f, scaleAlpha(kWaveShadow, opacity * 0.78f));
    drawing::fillStroke(canvas, wave, 1.35f, scaleAlpha(kOutputWave, opacity));
  }

  if (active_amount > 0.05f && draw_progress > 0.02f && draw_progress < 0.99f) {
    const float draw_x = plot.x + plot.width * (1.0f - draw_progress);
    drawing::drawLine(canvas,
                      draw_x,
                      plot.y + 2.0f,
                      draw_x,
                      plot.y + plot.height - 2.0f,
                      1.45f,
                      scaleAlpha(0xffbaf5c7, opacity * active_amount));
  }
}

void drawBackground(visage::Canvas& canvas, const Dimensions& dimensions) {
  canvas.setColor(kBackground);
  canvas.fill(0, 0, dimensions.width, dimensions.height);
}

struct BlockState {
  Rect rect;
  float audio_start_blocks = 0.0f;
  float generated_amount = 0.0f;
  float draw_progress = 0.0f;
  float active_amount = 0.0f;
  bool visible = false;
};

BlockState blockState(float age,
                      float local_cycle,
                      float audio_start_blocks,
                      float block_width,
                      float block_height,
                      float stream_y,
                      float process_y,
                      float input_x,
                      float intake_x) {
  if (age < 0.0f)
    return {};

  float x = lerp(input_x, intake_x, ease(interval(age, 0.0f, kApproachEndAge)));
  float y = stream_y;
  float generated_amount = 0.0f;
  float draw_progress = 0.0f;
  float active_amount = 0.0f;
  const float output_slot_x = intake_x + local_cycle * block_width;

  if (age >= kApproachEndAge && age < kHoldBeforeLiftEndAge) {
    x = intake_x;
    active_amount = 0.20f;
  }
  else if (age >= kHoldBeforeLiftEndAge && age < kLiftEndAge) {
    const float t = ease(interval(age, kHoldBeforeLiftEndAge, kLiftEndAge));
    x = intake_x;
    y = lerp(stream_y, process_y, t);
    active_amount = lerp(0.20f, 1.0f, t);
  }
  else if (age >= kLiftEndAge && age < kProcessEndAge) {
    const float t = ease(interval(age, kLiftEndAge, kProcessEndAge));
    x = intake_x;
    y = process_y;
    generated_amount = t;
    draw_progress = t;
    active_amount = 1.0f;
  }
  else if (age >= kProcessEndAge && age < kDropEndAge) {
    const float t = ease(interval(age, kProcessEndAge, kDropEndAge));
    x = lerp(intake_x, output_slot_x, t);
    y = lerp(process_y, stream_y, t);
    generated_amount = 1.0f;
    draw_progress = 1.0f;
    active_amount = lerp(1.0f, 0.20f, t);
  }
  else if (age >= kDropEndAge) {
    x = output_slot_x;
    y = stream_y;
    generated_amount = 1.0f;
    draw_progress = 1.0f;
  }

  return {
      { x, y, block_width, block_height },
      audio_start_blocks,
      generated_amount,
      draw_progress,
      active_amount,
      true,
  };
}

void drawGeneratedOutputChain(visage::Canvas& canvas,
                              float leading_block,
                              float local_cycle,
                              float intake_x,
                              float block_width,
                              float block_height,
                              float stream_y,
                              float view_right) {
  for (int slot = 1; slot <= 16; ++slot) {
    const float slot_offset = static_cast<float>(slot);
    const float audio_start_blocks = leading_block - slot_offset;
    const float x = intake_x + (local_cycle + slot_offset) * block_width;
    if (x >= view_right + block_width)
      continue;

    const Rect block { x, stream_y, block_width, block_height };
    drawBlock(canvas, block, audio_start_blocks, 1.0f, 1.0f, 0.0f, 0.96f);
  }
}

void drawEndpointLabels(visage::Canvas& canvas, float y, float view_width) {
  constexpr float kLabelWidth = 178.0f;
  constexpr float kLabelHeight = 32.0f;
  constexpr float kLabelSize = 20.5f;
  constexpr uint32_t kEndpointLabelColor = 0xffb8beb9;

  drawing::fauxBoldText(canvas,
                        "EMPTY BUFFER",
                        kLabelSize,
                        kEndpointLabelColor,
                        visage::Font::kCenter,
                        34.0f,
                        y,
                        kLabelWidth,
                        kLabelHeight);
  drawing::fauxBoldText(canvas,
                        "OUTPUT",
                        kLabelSize,
                        kEndpointLabelColor,
                        visage::Font::kCenter,
                        view_width - kLabelWidth - 34.0f,
                        y,
                        kLabelWidth,
                        kLabelHeight);
}

void drawOscillatorFactory(DrawContext& context,
                           const Dimensions& dimensions,
                           const Timeline& timeline,
                           const OscillatorBlockFactoryOptions& options) {
  visage::Canvas& canvas = context.canvas;
  const float w = static_cast<float>(dimensions.width);
  const float h = static_cast<float>(dimensions.height);
  const float center_x = w * 0.5f;

  const float node_scale = 0.50f;
  const float node_w = 500.0f * node_scale;
  const float node_h = 357.0f * node_scale;
  const float node_x = center_x - node_w * 0.5f;

  const float block_width = 92.0f;
  const float block_height = 48.0f;
  const float node_to_stream_gap = 14.0f;
  constexpr float kLabelBandHeight = 40.0f;
  const float scene_visual_height = kLabelBandHeight + node_h + node_to_stream_gap + block_height;
  const float node_y =
      std::max(kLabelBandHeight, (h - scene_visual_height) * 0.5f + kLabelBandHeight);
  const float intake_x = center_x - block_width * 0.5f;
  const float stream_y = node_y + node_h + node_to_stream_gap;
  const float process_y = node_y + 84.0f * node_scale + (231.0f * node_scale - block_height) * 0.5f;
  const float input_x = -block_width - 72.0f;

  if (options.clear_background)
    drawBackground(canvas, dimensions);

  drawEndpointLabels(canvas, node_y - 26.0f, w);

  const double normalized_loop =
      timeline.normalized_time - std::floor(timeline.normalized_time);
  const float loop_phase_blocks =
      kLoopStartOffsetBlocks + static_cast<float>(normalized_loop) * kLoopBlockCycles;
  const float leading_block = std::floor(loop_phase_blocks);
  const float local_cycle = loop_phase_blocks - leading_block;
  const float age = local_cycle * kLoopAge;

  drawGeneratedOutputChain(
      canvas, leading_block, local_cycle, intake_x, block_width, block_height, stream_y, w);

  HiseNodeContainerOptions node_options;
  node_options.label = "OSC";
  drawHiseNodeContainerAt(context, node_options, node_x, node_y, node_scale);

  const BlockState block = blockState(age,
                                      local_cycle,
                                      leading_block,
                                      block_width,
                                      block_height,
                                      stream_y,
                                      process_y,
                                      input_x,
                                      intake_x);
  if (block.visible && block.rect.x + block.rect.width > -4.0f && block.rect.x < w + 4.0f) {
    drawBlock(canvas,
              block.rect,
              block.audio_start_blocks,
              block.generated_amount,
              block.draw_progress,
              block.active_amount);
  }
}

} // namespace

void drawOscillatorBlockFactoryGraphic(DrawContext& context,
                                       const Dimensions& dimensions,
                                       const Timeline& timeline,
                                       const OscillatorBlockFactoryOptions& options) {
  drawOscillatorFactory(context, dimensions, timeline, options);
}

} // namespace adt::canonical::renderers
