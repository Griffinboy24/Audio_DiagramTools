#include "audio_diagram_tools/canonical_components.h"

#include <cstdint>
#include <iostream>

namespace {

bool hasVisibleVariation(const visage::Screenshot& screenshot) {
  const uint8_t* data = screenshot.data();
  const int bytes = screenshot.width() * screenshot.height() * 4;
  int changed_samples = 0;

  for (int i = 4; i < bytes; i += 113) {
    if (data[i] != data[0] || data[i + 1] != data[1] || data[i + 2] != data[2])
      ++changed_samples;
  }

  return changed_samples > 64;
}

} // namespace

int main() {
  const auto scene_component = adt::canonical::audioFileToSpeakerScene();
  const auto scene_metadata = adt::canonical::canonicalGraphicById(scene_component.canonical_id);
  if (!scene_metadata) {
    std::cerr << "Canonical scene lookup failed.\n";
    return 1;
  }

  const adt::Timeline timeline = adt::Timeline::forFrame(12, 90, 18.0);
  const visage::Screenshot scene = adt::canonical::renderCanonicalGraphicFrame(
      scene_component.canonical_id, scene_component.dimensions, timeline, scene_component.options);

  if (scene.width() != scene_component.dimensions.width ||
      scene.height() != scene_component.dimensions.height) {
    std::cerr << "Unexpected canonical scene dimensions.\n";
    return 1;
  }

  if (!hasVisibleVariation(scene)) {
    std::cerr << "Canonical scene appears blank.\n";
    return 1;
  }

  adt::canonical::AudioFilePlayerOptions player_options;
  player_options.playhead_progress = 0.35f;
  player_options.erase_sweep = false;
  const auto player_component =
      adt::canonical::audioFilePlayerGraphic(player_options, { 420, 188 });
  const visage::Screenshot player = adt::canonical::renderCanonicalGraphicFrame(
      player_component.canonical_id,
      player_component.dimensions,
      timeline,
      player_component.options);

  if (player.width() != 420 || player.height() != 188 || !hasVisibleVariation(player)) {
    std::cerr << "Canonical audio player render failed.\n";
    return 1;
  }

  return 0;
}
