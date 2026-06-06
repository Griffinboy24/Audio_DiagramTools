#include "audio_diagram_tools/waveform_scene.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace adt {
namespace {

constexpr float kPi = 3.14159265358979323846f;

struct PlotArea {
  float x = 0.0f;
  float y = 0.0f;
  float width = 0.0f;
  float height = 0.0f;
};

float clamp01(float value) {
  return std::max(0.0f, std::min(1.0f, value));
}

float amplitudeAt(float x_normalized, const Timeline& timeline, const WaveformSpec& waveform) {
  const float depth = clamp01(waveform.modulation_depth);
  const float phase = static_cast<float>(timeline.normalized_time);
  const float mod_phase = (x_normalized * waveform.modulation_cycles) - phase;
  const float unipolar_mod = 0.5f + 0.5f * std::sin(2.0f * kPi * mod_phase);
  return (1.0f - depth) + depth * unipolar_mod;
}

PlotArea plotArea(const Dimensions& dimensions, const Margins& margins) {
  PlotArea area;
  area.x = margins.left;
  area.y = margins.top;
  area.width = std::max(1.0f, static_cast<float>(dimensions.width) - margins.left - margins.right);
  area.height = std::max(1.0f, static_cast<float>(dimensions.height) - margins.top - margins.bottom);
  return area;
}

void fillStroke(visage::Canvas& canvas,
                const visage::Path& path,
                float width,
                visage::Path::Join join = visage::Path::Join::Round,
                visage::Path::EndCap cap = visage::Path::EndCap::Round,
                const std::vector<float>& dash = {},
                float dash_offset = 0.0f) {
  visage::Path stroke_source = path;
  canvas.fill(stroke_source.stroke(width, join, cap, dash, dash_offset));
}

void drawBackground(visage::Canvas& canvas, const Dimensions& dimensions, const RenderStyle& style) {
  canvas.setColor(visage::Brush::vertical(style.palette.background_top, style.palette.background_bottom));
  canvas.fill(0, 0, dimensions.width, dimensions.height);

  visage::Gradient accent_gradient(style.palette.accent_glow, 0x00000000);
  canvas.setColor(visage::Brush::radial(accent_gradient,
                                        visage::Point(dimensions.width * 0.74f,
                                                      dimensions.height * 0.28f),
                                        dimensions.height * 0.72f));
  canvas.fill(0, 0, dimensions.width, dimensions.height);
}

void drawGrid(visage::Canvas& canvas, const PlotArea& area, const RenderStyle& style) {
  canvas.setColor(style.palette.panel);
  canvas.roundedRectangle(area.x - 24.0f, area.y - 24.0f, area.width + 48.0f, area.height + 48.0f, 8.0f);

  const int vertical_divisions = 16;
  const int horizontal_divisions = 8;

  for (int i = 0; i <= vertical_divisions; ++i) {
    const bool major = i % 4 == 0;
    const float x = area.x + area.width * static_cast<float>(i) / static_cast<float>(vertical_divisions);
    canvas.setColor(major ? style.palette.grid_major : style.palette.grid_minor);
    canvas.fill(x, area.y, major ? style.grid_line_width + 0.5f : style.grid_line_width, area.height);
  }

  for (int i = 0; i <= horizontal_divisions; ++i) {
    const bool major = i % 2 == 0;
    const float y = area.y + area.height * static_cast<float>(i) / static_cast<float>(horizontal_divisions);
    canvas.setColor(major ? style.palette.grid_major : style.palette.grid_minor);
    canvas.fill(area.x, y, area.width, major ? style.grid_line_width + 0.5f : style.grid_line_width);
  }

  canvas.setColor(style.palette.axis);
  canvas.fill(area.x, area.y + area.height * 0.5f - style.axis_line_width * 0.5f,
              area.width, style.axis_line_width);
}

visage::Path buildWavePath(const PlotArea& area, const Timeline& timeline, const WaveformSpec& waveform) {
  const int samples = std::max(320, static_cast<int>(area.width));
  const float center_y = area.y + area.height * 0.5f;
  const float amplitude_pixels = area.height * 0.42f;
  const float phase = static_cast<float>(timeline.normalized_time) + waveform.phase_offset_cycles;

  visage::Path path;
  for (int i = 0; i < samples; ++i) {
    const float t = static_cast<float>(i) / static_cast<float>(samples - 1);
    const float carrier = std::sin(2.0f * kPi * (t * waveform.carrier_cycles + phase));
    const float amplitude = amplitudeAt(t, timeline, waveform);
    const float x = area.x + t * area.width;
    const float y = center_y - carrier * amplitude * amplitude_pixels;

    if (i == 0)
      path.moveTo(x, y);
    else
      path.lineTo(x, y);
  }
  return path;
}

void buildEnvelopePaths(visage::Path& top,
                        visage::Path& bottom,
                        const PlotArea& area,
                        const Timeline& timeline,
                        const WaveformSpec& waveform) {
  const int samples = std::max(160, static_cast<int>(area.width / 2.0f));
  const float center_y = area.y + area.height * 0.5f;
  const float amplitude_pixels = area.height * 0.42f;

  for (int i = 0; i < samples; ++i) {
    const float t = static_cast<float>(i) / static_cast<float>(samples - 1);
    const float envelope = amplitudeAt(t, timeline, waveform) * amplitude_pixels;
    const float x = area.x + t * area.width;
    const float y_top = center_y - envelope;
    const float y_bottom = center_y + envelope;

    if (i == 0) {
      top.moveTo(x, y_top);
      bottom.moveTo(x, y_bottom);
    }
    else {
      top.lineTo(x, y_top);
      bottom.lineTo(x, y_bottom);
    }
  }
}

} // namespace

void drawVolumeModulatedSine(visage::Canvas& canvas,
                             const Dimensions& dimensions,
                             const Timeline& timeline,
                             const RenderStyle& style,
                             const WaveformSpec& waveform) {
  drawBackground(canvas, dimensions, style);

  const PlotArea area = plotArea(dimensions, style.margins);
  if (waveform.show_grid)
    drawGrid(canvas, area, style);

  if (waveform.show_envelope) {
    visage::Path envelope_top;
    visage::Path envelope_bottom;
    buildEnvelopePaths(envelope_top, envelope_bottom, area, timeline, waveform);

    const float dash_offset = static_cast<float>(timeline.frame_index) * 0.6f;
    canvas.setColor(style.palette.envelope_glow);
    fillStroke(canvas, envelope_top, style.envelope_width + 6.0f);
    fillStroke(canvas, envelope_bottom, style.envelope_width + 6.0f);

    canvas.setColor(style.palette.envelope);
    fillStroke(canvas, envelope_top, style.envelope_width, visage::Path::Join::Round,
               visage::Path::EndCap::Round, { 12.0f, 10.0f }, dash_offset);
    fillStroke(canvas, envelope_bottom, style.envelope_width, visage::Path::Join::Round,
               visage::Path::EndCap::Round, { 12.0f, 10.0f }, dash_offset);
  }

  const visage::Path wave = buildWavePath(area, timeline, waveform);
  canvas.setColor(style.palette.waveform_glow);
  fillStroke(canvas, wave, style.waveform_glow_width);

  canvas.setColor(style.palette.waveform);
  fillStroke(canvas, wave, style.waveform_width);

  canvas.setColor(style.palette.waveform_highlight);
  fillStroke(canvas, wave, style.waveform_highlight_width);
}

visage::Screenshot renderVolumeModulatedSineFrame(const Dimensions& dimensions,
                                                  const Timeline& timeline,
                                                  const RenderStyle& style,
                                                  const WaveformSpec& waveform) {
  visage::Canvas canvas;
  canvas.setWindowless(dimensions.width, dimensions.height);
  canvas.updateTime(timeline.time_seconds);
  drawVolumeModulatedSine(canvas, dimensions, timeline, style, waveform);
  canvas.submit();
  return canvas.takeScreenshot();
}

void saveVolumeModulatedSineFrame(const std::string& output_path,
                                  const Dimensions& dimensions,
                                  const Timeline& timeline,
                                  const RenderStyle& style,
                                  const WaveformSpec& waveform) {
  renderVolumeModulatedSineFrame(dimensions, timeline, style, waveform).save(output_path);
}

} // namespace adt
