#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>

namespace adt {

struct Dimensions {
  int width = 1280;
  int height = 720;
};

struct CanvasPreset {
  std::string_view id;
  std::string_view description;
  Dimensions dimensions;
};

const std::array<CanvasPreset, 4>& canvasPresets();
std::optional<CanvasPreset> canvasPresetById(std::string_view id);
Dimensions dimensionsForPreset(std::string_view id);

struct Timeline {
  int frame_index = 0;
  int frame_count = 1;
  double fps = 30.0;
  double time_seconds = 0.0;
  double normalized_time = 0.0;

  static Timeline forFrame(int frame_index, int frame_count, double fps);
};

struct Margins {
  float left = 96.0f;
  float top = 88.0f;
  float right = 96.0f;
  float bottom = 104.0f;
};

struct Palette {
  uint32_t background_top = 0xff101418;
  uint32_t background_bottom = 0xff18131d;
  uint32_t panel = 0x22171d24;
  uint32_t grid_minor = 0x18ffffff;
  uint32_t grid_major = 0x32ffffff;
  uint32_t axis = 0x66ffffff;
  uint32_t envelope = 0xfff6c95f;
  uint32_t envelope_glow = 0x44f6c95f;
  uint32_t waveform = 0xff62e4ff;
  uint32_t waveform_highlight = 0xffffffff;
  uint32_t waveform_glow = 0x5562e4ff;
  uint32_t accent = 0xffff5c8a;
  uint32_t accent_glow = 0x33ff5c8a;
};

struct RenderStyle {
  Palette palette;
  Margins margins;
  float grid_line_width = 1.0f;
  float axis_line_width = 2.0f;
  float waveform_width = 4.0f;
  float waveform_highlight_width = 1.2f;
  float waveform_glow_width = 14.0f;
  float envelope_width = 2.0f;
};

struct WaveformSpec {
  float carrier_cycles = 7.0f;
  float modulation_cycles = 1.0f;
  float modulation_depth = 0.72f;
  float phase_offset_cycles = 0.0f;
  bool show_grid = true;
  bool show_envelope = true;
};

} // namespace adt
