#include "audio_diagram_tools/canonical_renderers.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace adt::canonical::renderers {
using namespace drawing;
namespace {

constexpr float kCyclesAcrossPlot = 3.35f;
constexpr float kSineAmplitude = 1.0f;

struct VolumeScaleLayout {
  Rect outer;
  Rect bevel;
  Rect content;
  Rect plot;
  float scale = 1.0f;
};

float volumeGain(const Timeline& timeline) {
  const float phase = static_cast<float>(timeline.normalized_time) -
                      std::floor(static_cast<float>(timeline.normalized_time));
  const float triangle = phase < 0.5f ? phase * 2.0f : (1.0f - phase) * 2.0f;
  return 0.48f + 0.52f * triangle;
}

std::vector<SignalPoint> makeScaledSineWaveform(const Rect& plot, int samples, float gain) {
  std::vector<SignalPoint> points;
  points.reserve(static_cast<size_t>(samples));

  const float center_y = plot.y + plot.height * 0.5f;
  const float amplitude = plot.height * 0.50f;
  for (int i = 0; i < samples; ++i) {
    const float t = static_cast<float>(i) / static_cast<float>(samples - 1);
    const float phase = 2.0f * kPi * kCyclesAcrossPlot * t + 0.28f;
    const float contour = std::sin(phase) + 0.08f * std::sin(3.0f * phase + 0.22f);
    const float value = gain * kSineAmplitude * contour / 0.9261f;
    points.push_back({ t, plot.x + plot.width * t, center_y - value * amplitude, value });
  }

  return points;
}

VolumeScaleLayout makeLayout(const Dimensions& dimensions) {
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

  const Rect outer { sx(76.0f), sy(58.0f), 768.0f * scale, 224.0f * scale };
  const Rect bevel = insetRect(outer, 3.6f * scale, 3.6f * scale);
  const Rect content = insetRect(bevel, 3.4f * scale, 3.4f * scale);
  const Rect plot { content.x + 26.0f * scale,
                    content.y + 34.0f * scale,
                    content.width - 52.0f * scale,
                    content.height - 68.0f * scale };
  return { outer, bevel, content, plot, scale };
}

} // namespace

void drawWaveformVolumeScaleGraphic(DrawContext& context,
                                    const Dimensions& dimensions,
                                    const Timeline& timeline) {
  visage::Canvas& canvas = context.canvas;
  canvas.setColor(0xffffffff);
  canvas.fill(0, 0, dimensions.width, dimensions.height);

  const VolumeScaleLayout layout = makeLayout(dimensions);
  const float radius = layout.outer.height * 0.052f;

  visage::Region& shadow = addBlurRegion(context, 8.0f * layout.scale);
  drawInRegion(context, shadow, [&](visage::Canvas& shadow_canvas) {
    fillRoundedRectPath(shadow_canvas,
                        layout.outer.x + 7.0f * layout.scale,
                        layout.outer.y + 10.0f * layout.scale,
                        layout.outer.width - 14.0f * layout.scale,
                        layout.outer.height,
                        radius,
                        0x25040a10);
  });

  DiagramFrameLayout frame;
  frame.outer = layout.outer;
  frame.bevel = layout.bevel;
  frame.content = layout.content;
  frame.plot = layout.plot;
  frame.radius = radius;
  drawDiagramFrame(canvas, frame);
  drawTimelineGrid(canvas, layout.plot, 8);
  drawTimelineZeroAxis(canvas, layout.plot);

  const std::vector<SignalPoint> waveform =
      makeScaledSineWaveform(layout.plot, 460, volumeGain(timeline));
  drawGriffinWaveformTrace(context, waveform, layout.plot, layout.scale);

  drawDiagramFrameCorners(canvas, frame);
}

} // namespace adt::canonical::renderers
