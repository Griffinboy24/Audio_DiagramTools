#include "audio_diagram_tools/canonical_renderers.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <vector>

namespace adt::canonical::renderers {
using namespace drawing;
namespace {

constexpr int kBufferCount = 3;

struct BufferBlock {
  Rect outer;
  Rect content;
  Rect plot;
};

struct BufferSplitLayout {
  std::array<BufferBlock, kBufferCount> blocks;
  float scale = 1.0f;
};

float sourceSignal(float t) {
  constexpr float kBoundary = 1.0f / 3.0f;
  constexpr float kSlowCycles = 3.8f;
  constexpr float kFastExtraCycles = 34.0f;
  constexpr float kFrequencyDecay = 8.0f;
  constexpr float kSettledCyclesPerSpan = 4.24f;
  auto settledPhaseCycles = [](float x) {
    return kSlowCycles * x +
           kFastExtraCycles * (1.0f - std::exp(-kFrequencyDecay * x)) /
               kFrequencyDecay +
           0.17f;
  };

  const float boundary_phase = settledPhaseCycles(kBoundary);
  const float phase_cycles = t < kBoundary ?
      boundary_phase - kSettledCyclesPerSpan * (kBoundary - t) -
          1.75f * (kBoundary - t) * (kBoundary - t) :
      boundary_phase + kSettledCyclesPerSpan * (t - kBoundary);
  const float phase = 2.0f * kPi * phase_cycles;
  const float tone_t = t < kBoundary ? kBoundary + t * 0.28f : t;
  float envelope =
      0.80f +
      0.11f * std::sin(2.0f * kPi * (1.13f * tone_t + 0.31f)) +
      0.035f * std::sin(2.0f * kPi * (3.40f * tone_t + 0.08f));
  const float chunk3_taper = smoothstep(2.0f / 3.0f, 0.735f, t);
  const float chunk3_first_hump =
      chunk3_taper * std::exp(-std::pow((t - 0.765f) / 0.070f, 2.0f));
  envelope *= 1.0f - 0.08f * chunk3_taper - 0.10f * chunk3_first_hump;
  const float roughness = 1.0f - 0.82f * smoothstep(0.04f, 0.48f, tone_t);
  const float contour =
      std::sin(phase) +
      (0.25f * roughness + 0.035f) * std::sin(2.0f * phase + 1.10f) +
      0.085f * roughness * std::sin(2.0f * kPi * (43.0f * t + 0.27f)) +
      0.040f * roughness * std::sin(2.0f * kPi * (71.0f * t + 0.41f));

  return std::clamp(envelope * contour / 1.38f, -0.96f, 0.96f);
}

std::vector<SignalPoint> makeBufferWaveform(const Rect& plot,
                                            int buffer_index,
                                            int samples) {
  std::vector<SignalPoint> points;
  points.reserve(static_cast<size_t>(samples));

  const float center_y = plot.y + plot.height * 0.5f;
  const float amplitude = plot.height * 0.49f;
  for (int i = 0; i < samples; ++i) {
    const float local_t = static_cast<float>(i) / static_cast<float>(samples - 1);
    const float global_t = (static_cast<float>(buffer_index) + local_t) /
                           static_cast<float>(kBufferCount);
    const float value = sourceSignal(global_t);
    points.push_back({ global_t,
                       plot.x + plot.width * local_t,
                       center_y - value * amplitude,
                       value });
  }

  return points;
}

void drawBlockGrid(visage::Canvas& canvas, const Rect& plot, float scale) {
  canvas.setColor(0xff18232f);
  constexpr int kHorizontalRows = 4;
  for (int i = 0; i <= kHorizontalRows; ++i) {
    const float y = plot.y + plot.height * static_cast<float>(i) /
                                 static_cast<float>(kHorizontalRows);
    canvas.fill(plot.x, y, plot.width, 1.05f * scale);
  }

  constexpr int kLocalColumns = 4;
  for (int i = 0; i <= kLocalColumns; ++i) {
    const float x = plot.x + plot.width * static_cast<float>(i) /
                                 static_cast<float>(kLocalColumns);
    canvas.fill(x, plot.y, 0.95f * scale, plot.height);
  }
}

void drawBufferBlock(DrawContext& context,
                     const BufferBlock& block,
                     float scale,
                     int buffer_index) {
  visage::Canvas& canvas = context.canvas;
  DiagramFrameLayout frame;
  frame.outer = block.outer;
  frame.bevel = insetRect(block.outer, 2.2f * scale, 2.2f * scale);
  frame.content = block.content;
  frame.plot = block.plot;
  frame.radius = 6.0f * scale;

  visage::Region& shadow = addBlurRegion(context, 4.5f * scale);
  drawInRegion(context, shadow, [&](visage::Canvas& shadow_canvas) {
    fillRoundedRectPath(shadow_canvas,
                        block.outer.x + 3.5f * scale,
                        block.outer.y + 6.0f * scale,
                        block.outer.width - 7.0f * scale,
                        block.outer.height - 2.0f * scale,
                        frame.radius,
                        0x18040a10);
  });

  fillRoundedRectPath(canvas,
                      block.outer.x,
                      block.outer.y,
                      block.outer.width,
                      block.outer.height,
                      frame.radius,
                      0xff071016);
  fillRoundedRectPath(canvas,
                      frame.bevel.x,
                      frame.bevel.y,
                      frame.bevel.width,
                      frame.bevel.height,
                      frame.radius - 1.2f * scale,
                      visage::Brush::vertical(0xff27313d, 0xff121820));
  fillRoundedRectPath(canvas,
                      block.content.x,
                      block.content.y,
                      block.content.width,
                      block.content.height,
                      frame.radius - 2.4f * scale,
                      visage::Brush::vertical(0xff171f2c, 0xff0b1118));
  canvas.setColor(buffer_index % 2 == 0 ? 0x105f83dc : 0x0a5f83dc);
  canvas.fill(block.content.x, block.content.y, block.content.width, block.content.height);

  drawBlockGrid(canvas, block.plot, scale);
  drawTimelineZeroAxis(canvas, block.plot);

  const std::vector<SignalPoint> waveform =
      makeBufferWaveform(block.plot, buffer_index, 620);
  drawGriffinWaveformTrace(context, waveform, block.plot, scale);

  drawDiagramFrameCorners(canvas, frame);
}

BufferSplitLayout makeLayout(const Dimensions& dimensions) {
  constexpr float kReferenceWidth = 920.0f;
  constexpr float kReferenceHeight = 340.0f;
  const float scale = std::min(static_cast<float>(dimensions.width) / kReferenceWidth,
                               static_cast<float>(dimensions.height) / kReferenceHeight);
  const float origin_x =
      (static_cast<float>(dimensions.width) - kReferenceWidth * scale) * 0.5f;
  const float origin_y =
      (static_cast<float>(dimensions.height) - kReferenceHeight * scale) * 0.5f;
  auto sx = [&](float x) { return origin_x + x * scale; };
  auto sy = [&](float y) { return origin_y + y * scale; };

  constexpr float kBlockWidth = 264.0f;
  constexpr float kBlockHeight = 166.0f;
  constexpr float kGap = 10.0f;
  constexpr float kTotalWidth = kBlockWidth * kBufferCount + kGap * (kBufferCount - 1);
  constexpr float kStartX = (kReferenceWidth - kTotalWidth) * 0.5f;
  constexpr float kTopY = 92.0f;

  BufferSplitLayout layout;
  layout.scale = scale;

  for (int i = 0; i < kBufferCount; ++i) {
    const float x = sx(kStartX + static_cast<float>(i) * (kBlockWidth + kGap));
    const Rect outer { x, sy(kTopY), kBlockWidth * scale, kBlockHeight * scale };
    const Rect content = insetRect(outer, 6.2f * scale, 6.2f * scale);
    const Rect plot { content.x,
                      content.y + 26.0f * scale,
                      content.width,
                      content.height - 52.0f * scale };
    layout.blocks[static_cast<size_t>(i)] = { outer, content, plot };
  }

  return layout;
}

void drawLabels(visage::Canvas& canvas, const BufferSplitLayout& layout) {
  const std::array<std::string, kBufferCount> labels {
    "Chunk 1",
    "Chunk 2",
    "Chunk 3",
  };

  for (int i = 0; i < kBufferCount; ++i) {
    const BufferBlock& block = layout.blocks[static_cast<size_t>(i)];
    drawDiagramPanelLabel(canvas, labels[static_cast<size_t>(i)], block.outer, layout.scale);
  }
}

} // namespace

void drawWaveformBufferSplitGraphic(DrawContext& context, const Dimensions& dimensions) {
  visage::Canvas& canvas = context.canvas;
  canvas.setColor(0xffffffff);
  canvas.fill(0, 0, dimensions.width, dimensions.height);

  const BufferSplitLayout layout = makeLayout(dimensions);
  for (int i = 0; i < kBufferCount; ++i)
    drawBufferBlock(context, layout.blocks[static_cast<size_t>(i)], layout.scale, i);

  drawLabels(canvas, layout);
}

} // namespace adt::canonical::renderers
