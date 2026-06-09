#include "audio_diagram_tools/render_types.h"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace adt {
namespace {

const std::array<CanvasPreset, 5> kCanvasPresets = {
  CanvasPreset { "blog-wide", "Default 16:9 blog diagram", { 1280, 720 } },
  CanvasPreset { "blog-banner", "Wide banner for inline article diagrams", { 1440, 420 } },
  CanvasPreset { "blog-wide-large", "Higher-resolution 16:9 source render", { 1600, 900 } },
  CanvasPreset { "gif-preview", "Compact 16:9 animated GIF", { 960, 540 } },
  CanvasPreset { "square", "Square social/blog thumbnail", { 1080, 1080 } },
};

} // namespace

const std::array<CanvasPreset, 5>& canvasPresets() {
  return kCanvasPresets;
}

std::optional<CanvasPreset> canvasPresetById(std::string_view id) {
  const auto& presets = canvasPresets();
  const auto it = std::find_if(presets.begin(), presets.end(), [id](const CanvasPreset& preset) {
    return preset.id == id;
  });

  if (it == presets.end())
    return std::nullopt;

  return *it;
}

Dimensions dimensionsForPreset(std::string_view id) {
  const auto preset = canvasPresetById(id);
  if (!preset)
    throw std::runtime_error("Unknown canvas preset: " + std::string(id));

  return preset->dimensions;
}

Timeline Timeline::forFrame(int frame_index, int frame_count, double fps) {
  Timeline timeline;
  timeline.frame_count = std::max(1, frame_count);
  timeline.frame_index = std::max(0, std::min(frame_index, timeline.frame_count - 1));
  timeline.fps = fps > 0.0 ? fps : 30.0;
  timeline.time_seconds = static_cast<double>(timeline.frame_index) / timeline.fps;
  timeline.normalized_time = static_cast<double>(timeline.frame_index) /
                             static_cast<double>(timeline.frame_count);
  return timeline;
}

} // namespace adt
