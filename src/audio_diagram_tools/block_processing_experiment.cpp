#include "audio_diagram_tools/block_processing_experiment.h"

#include "audio_diagram_tools/canonical_components.h"
#include "audio_diagram_tools/canonical_drawing.h"
#include "audio_diagram_tools/canonical_renderers.h"
#include "audio_diagram_tools/png_export.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>

namespace adt::experiments {
namespace {

using canonical::drawing::DrawContext;
using canonical::drawing::Rect;

constexpr float kPi = canonical::drawing::kPi;

constexpr uint32_t kBackground = 0xff191919;
constexpr uint32_t kBackgroundRim = 0xff242424;
constexpr uint32_t kBlockFillTop = 0xff333333;
constexpr uint32_t kBlockFillBottom = 0xff242424;
constexpr uint32_t kProcessedFillTop = 0xff2d3a33;
constexpr uint32_t kProcessedFillBottom = 0xff202822;
constexpr uint32_t kActiveFillTop = 0xff3b4640;
constexpr uint32_t kActiveFillBottom = 0xff29312d;
constexpr uint32_t kBlockOuterStroke = 0xff0c0c0c;
constexpr uint32_t kBlockBevel = 0xff5c5c5c;
constexpr uint32_t kProcessedBevel = 0xff607265;
constexpr uint32_t kActiveBevel = 0xff7acd93;
constexpr uint32_t kInputWave = 0xffa7aaa8;
constexpr uint32_t kOutputWave = 0xff86c799;
constexpr uint32_t kWaveShadow = 0xff101410;
constexpr uint32_t kBlockMidline = 0xff474747;
constexpr uint32_t kBlockTick = 0xff3a3a3a;
constexpr uint32_t kActiveGlow = 0xff7acd93;

constexpr float kWavePeriodBlocks = 8.0f;
constexpr float kLoopAge = 6.0f;
constexpr float kLoopBlockCycles = kWavePeriodBlocks;
constexpr float kLoopStartOffsetBlocks = 0.43f;
constexpr float kApproachEndAge = 1.45f;
constexpr float kHoldBeforeLiftEndAge = 1.62f;
constexpr float kLiftEndAge = 2.16f;
constexpr float kProcessEndAge = 3.14f;
constexpr float kDropEndAge = 3.74f;

uint8_t channel(uint32_t color, int shift) {
  return static_cast<uint8_t>((color >> shift) & 0xffu);
}

uint32_t scaleAlpha(uint32_t color, float alpha_scale) {
  const uint8_t alpha = static_cast<uint8_t>(std::lround(
      static_cast<float>(channel(color, 24)) * canonical::drawing::clamp01(alpha_scale)));
  return (static_cast<uint32_t>(alpha) << 24) | (color & 0x00ffffffu);
}

uint32_t lerpColor(uint32_t a, uint32_t b, float t) {
  t = canonical::drawing::clamp01(t);
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
  return canonical::drawing::clamp01(value);
}

float ease(float value) {
  return canonical::drawing::smoothstep(0.0f, 1.0f, clamp01(value));
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

float waveInput(float x_blocks) {
  const float x = wrapBlockCoordinate(x_blocks) / kWavePeriodBlocks;
  const float main = std::sin(2.0f * kPi * (x * 3.0f + 0.08f));
  const float texture = 0.18f * std::sin(2.0f * kPi * (x * 7.0f - 0.17f));
  const float motion = 0.08f * std::sin(2.0f * kPi * (x * 11.0f + 0.34f));
  return std::clamp(main * 0.74f + texture + motion, -0.92f, 0.92f);
}

float waveProcessed(float x_blocks) {
  const float x = wrapBlockCoordinate(x_blocks) / kWavePeriodBlocks;
  const float clean = std::sin(2.0f * kPi * (x * 3.0f + 0.08f));
  return std::clamp(std::tanh(clean * 1.16f) * 0.82f, -0.90f, 0.90f);
}

float mixedWave(float stream_x_blocks, float processed_mix) {
  return lerp(waveInput(stream_x_blocks), waveProcessed(stream_x_blocks), processed_mix);
}

visage::Path blockWavePath(const Rect& block, float audio_start_blocks, float processed_mix) {
  const Rect plot { block.x + 7.0f, block.y + 9.0f, block.width - 14.0f, block.height - 18.0f };
  const float center_y = plot.y + plot.height * 0.5f;
  const float amplitude = plot.height * 0.39f;
  const int samples = 60;

  visage::Path path;
  for (int i = 0; i < samples; ++i) {
    const float local = static_cast<float>(i) / static_cast<float>(samples - 1);
    const float x = plot.x + plot.width * local;
    const float value = mixedWave(audio_start_blocks + local, processed_mix);
    const float y = center_y - value * amplitude;

    if (i == 0)
      path.moveTo(x, y);
    else
      path.lineTo(x, y);
  }

  return path;
}

void drawBlock(visage::Canvas& canvas,
               const Rect& block,
               float audio_start_blocks,
               float processed_mix,
               float active_amount,
               float opacity = 1.0f) {
  opacity = clamp01(opacity);
  if (opacity <= 0.0f)
    return;

  const float emphasis = std::max(active_amount, processed_mix * 0.45f);
  const uint32_t fill_top = scaleAlpha(
      lerpColor(lerpColor(kBlockFillTop, kProcessedFillTop, processed_mix),
                kActiveFillTop,
                active_amount),
      opacity);
  const uint32_t fill_bottom = scaleAlpha(
      lerpColor(lerpColor(kBlockFillBottom, kProcessedFillBottom, processed_mix),
                kActiveFillBottom,
                active_amount),
      opacity);
  const uint32_t bevel = scaleAlpha(
      lerpColor(lerpColor(kBlockBevel, kProcessedBevel, processed_mix), kActiveBevel, emphasis),
      opacity);
  const uint32_t wave = scaleAlpha(lerpColor(kInputWave, kOutputWave, processed_mix), opacity);
  const uint32_t wave_shadow = scaleAlpha(kWaveShadow, opacity * 0.78f);
  const float radius = 2.75f;

  if (active_amount > 0.03f) {
    canvas.setColor(scaleAlpha(kActiveGlow, active_amount * opacity * 0.13f));
    canvas.roundedRectangle(
        block.x - 3.0f, block.y - 3.0f, block.width + 6.0f, block.height + 6.0f, radius + 2.0f);
  }

  canvas.setColor(scaleAlpha(0xff000000, opacity * 0.22f));
  canvas.roundedRectangle(block.x + 2.0f, block.y + 2.0f, block.width, block.height, radius);

  canvas.setColor(scaleAlpha(kBlockOuterStroke, opacity));
  canvas.roundedRectangle(block.x, block.y, block.width, block.height, radius);
  canvas.setColor(visage::Brush::vertical(fill_top, fill_bottom));
  canvas.roundedRectangle(
      block.x + 1.0f, block.y + 1.0f, block.width - 2.0f, block.height - 2.0f, radius - 0.75f);
  canvas.setColor(bevel);
  canvas.roundedRectangleBorder(block.x + 0.5f,
                                block.y + 0.5f,
                                block.width - 1.0f,
                                block.height - 1.0f,
                                radius,
                                1.0f);
  canvas.setColor(scaleAlpha(0xff0b0b0b, opacity * 0.68f));
  canvas.roundedRectangleBorder(block.x + 3.0f,
                                block.y + 3.0f,
                                block.width - 6.0f,
                                block.height - 6.0f,
                                radius - 1.2f,
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

  canonical::drawing::drawLine(canvas,
                               plot.x + 1.0f,
                               center_y,
                               plot.x + plot.width - 1.0f,
                               center_y,
                               0.65f,
                               scaleAlpha(kBlockMidline, opacity * 0.82f));

  canvas.setColor(scaleAlpha(lerpColor(0xff4b4b4b, kActiveGlow, processed_mix), opacity * 0.92f));
  canvas.fill(block.x + 5.0f, block.y + 6.0f, 2.2f, block.height - 12.0f);
  canvas.setColor(scaleAlpha(kBlockOuterStroke, opacity * 0.78f));
  canvas.fill(block.x + block.width - 6.5f, block.y + 6.0f, 1.3f, block.height - 12.0f);

  const visage::Path wave_path = blockWavePath(block, audio_start_blocks, processed_mix);
  canonical::drawing::fillStroke(canvas, wave_path, 2.9f, wave_shadow);
  canonical::drawing::fillStroke(canvas, wave_path, 1.35f, wave);
}

void drawBackground(visage::Canvas& canvas, const Dimensions& dimensions) {
  canvas.setColor(kBackground);
  canvas.fill(0, 0, dimensions.width, dimensions.height);

  canvas.setColor(0x26000000);
  canvas.rectangle(0.0f, 0.0f, static_cast<float>(dimensions.width), 26.0f);
  canvas.rectangle(0.0f,
                   static_cast<float>(dimensions.height) - 30.0f,
                   static_cast<float>(dimensions.width),
                   30.0f);
  canvas.setColor(kBackgroundRim);
  canvas.rectangle(0.0f, 0.0f, static_cast<float>(dimensions.width), 1.0f);
  canvas.rectangle(0.0f,
                   static_cast<float>(dimensions.height) - 1.0f,
                   static_cast<float>(dimensions.width),
                   1.0f);
}

struct BlockState {
  Rect rect;
  float audio_start_blocks = 0.0f;
  float processed_mix = 0.0f;
  float active_amount = 0.0f;
  bool visible = false;
};

BlockState blockState(float age,
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
  float processed_mix = 0.0f;
  float active_amount = 0.0f;

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
    processed_mix = t;
    active_amount = 1.0f;
  }
  else if (age >= kProcessEndAge && age < kDropEndAge) {
    const float t = ease(interval(age, kProcessEndAge, kDropEndAge));
    x = intake_x;
    y = lerp(process_y, stream_y, t);
    processed_mix = 1.0f;
    active_amount = lerp(1.0f, 0.20f, t);
  }
  else if (age >= kDropEndAge) {
    x = intake_x;
    y = stream_y;
    processed_mix = 1.0f;
  }

  return {
      { x, y, block_width, block_height },
      audio_start_blocks,
      processed_mix,
      active_amount,
      true,
  };
}

float outputPushProgress(float age) {
  if (age < kDropEndAge)
    return 0.0f;

  return ease(interval(age, kDropEndAge, kLoopAge));
}

void drawProcessedOutputChain(visage::Canvas& canvas,
                              float stream_phase_blocks,
                              float intake_x,
                              float block_width,
                              float block_height,
                              float stream_y,
                              float view_right) {
  const float leading_block = std::floor(stream_phase_blocks);
  for (int slot = 1; slot <= 16; ++slot) {
    const float audio_start_blocks = leading_block - static_cast<float>(slot);
    const float x = intake_x + (stream_phase_blocks - audio_start_blocks) * block_width;
    if (x >= view_right + block_width)
      continue;

    const Rect block { x, stream_y, block_width, block_height };
    drawBlock(canvas, block, audio_start_blocks, 1.0f, 0.0f, 0.96f);
  }
}

} // namespace

visage::Screenshot renderBlockProcessingExperimentFrame(const Dimensions& dimensions,
                                                        const Timeline& timeline) {
  visage::Canvas canvas;
  canvas.setWindowless(dimensions.width, dimensions.height);
  canvas.updateTime(timeline.time_seconds);
  DrawContext context(canvas, dimensions);

  const float w = static_cast<float>(dimensions.width);
  const float h = static_cast<float>(dimensions.height);
  const float center_x = w * 0.5f;

  const float node_scale = 0.50f;
  const float node_w = 500.0f * node_scale;
  const float node_h = 357.0f * node_scale;
  const float node_x = center_x - node_w * 0.5f;
  const float node_y = 43.0f;

  const float block_width = 92.0f;
  const float block_height = 48.0f;
  const float intake_x = center_x - block_width * 0.5f;
  const float stream_y = std::min(h - 82.0f, node_y + node_h + 14.0f);
  const float process_y = node_y + 84.0f * node_scale + (231.0f * node_scale - block_height) * 0.5f;
  const float input_x = -block_width - 72.0f;

  drawBackground(canvas, dimensions);

  const float loop_phase_blocks =
      kLoopStartOffsetBlocks + static_cast<float>(timeline.normalized_time) * kLoopBlockCycles;
  const float leading_block = std::floor(loop_phase_blocks);
  const float local_cycle = loop_phase_blocks - leading_block;
  const float age = local_cycle * kLoopAge;
  const float output_push = outputPushProgress(age);
  drawProcessedOutputChain(
      canvas, loop_phase_blocks, intake_x, block_width, block_height, stream_y, w);

  canonical::HiseNodeContainerOptions node_options;
  node_options.label = "";
  canonical::renderers::drawHiseNodeContainerAt(context, node_options, node_x, node_y, node_scale);

  const BlockState block =
      blockState(
          age, leading_block, block_width, block_height, stream_y, process_y, input_x, intake_x);
  if (block.visible && age >= kDropEndAge) {
    const Rect pushed_block {
      intake_x + output_push * block_width,
      block.rect.y,
      block.rect.width,
      block.rect.height,
    };
    drawBlock(canvas,
              pushed_block,
              block.audio_start_blocks,
              block.processed_mix,
              block.active_amount);
  }
  else if (block.visible && block.rect.x + block.rect.width > -4.0f && block.rect.x < w + 4.0f) {
    drawBlock(canvas,
              block.rect,
              block.audio_start_blocks,
              block.processed_mix,
              block.active_amount);
  }

  canvas.submit();
  return canvas.takeScreenshot();
}

void saveBlockProcessingExperimentFrame(const std::string& output_path,
                                        const Dimensions& dimensions,
                                        const Timeline& timeline) {
  savePngWithStraightAlpha(output_path,
                           renderBlockProcessingExperimentFrame(dimensions, timeline));
}

} // namespace adt::experiments
