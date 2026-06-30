#include "audio_diagram_tools/canonical_components.h"

#include <algorithm>
#include <cstdlib>
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

bool screenshotsNearlyMatch(const visage::Screenshot& a, const visage::Screenshot& b) {
  if (a.width() != b.width() || a.height() != b.height())
    return false;

  const uint8_t* a_data = a.data();
  const uint8_t* b_data = b.data();
  const int pixels = a.width() * a.height();
  uint64_t total_delta = 0;
  int max_delta = 0;

  for (int pixel = 0; pixel < pixels; ++pixel) {
    for (int channel = 0; channel < 3; ++channel) {
      const int delta = std::abs(static_cast<int>(a_data[pixel * 4 + channel]) -
                                 static_cast<int>(b_data[pixel * 4 + channel]));
      total_delta += static_cast<uint64_t>(delta);
      max_delta = std::max(max_delta, delta);
    }
  }

  const double mean_delta =
      static_cast<double>(total_delta) / static_cast<double>(pixels * 3);
  return max_delta <= 1 && mean_delta <= 0.01;
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

  const auto voice_scene_component = adt::canonical::voiceSampleToSpeakerScene();
  const auto voice_scene_metadata =
      adt::canonical::canonicalGraphicById(voice_scene_component.canonical_id);
  if (!voice_scene_metadata) {
    std::cerr << "Canonical voice sample scene lookup failed.\n";
    return 1;
  }

  const visage::Screenshot voice_scene = adt::canonical::renderCanonicalGraphicFrame(
      voice_scene_component.canonical_id,
      voice_scene_component.dimensions,
      timeline,
      voice_scene_component.options);

  if (voice_scene.width() != voice_scene_component.dimensions.width ||
      voice_scene.height() != voice_scene_component.dimensions.height ||
      !hasVisibleVariation(voice_scene)) {
    std::cerr << "Canonical voice sample scene render failed.\n";
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

  const auto block_component = adt::canonical::blockProcessingGraphic({}, { 920, 330 });
  const visage::Screenshot block = adt::canonical::renderCanonicalGraphicFrame(
      block_component.canonical_id,
      block_component.dimensions,
      timeline,
      block_component.options);

  if (block.width() != 920 || block.height() != 330 || !hasVisibleVariation(block)) {
    std::cerr << "Canonical block processing render failed.\n";
    return 1;
  }

  const auto annotated_block_component =
      adt::canonical::blockProcessingGraphic({}, { 920, 330 });
  adt::Timeline block_loop_start;
  block_loop_start.frame_count = 150;
  block_loop_start.fps = 15.0;
  block_loop_start.normalized_time = 0.0;
  adt::Timeline block_loop_end = block_loop_start;
  block_loop_end.frame_index = block_loop_start.frame_count;
  block_loop_end.time_seconds =
      static_cast<double>(block_loop_start.frame_count) / block_loop_start.fps;
  block_loop_end.normalized_time = 1.0;

  const visage::Screenshot block_loop_0 = adt::canonical::renderCanonicalGraphicFrame(
      annotated_block_component.canonical_id,
      annotated_block_component.dimensions,
      block_loop_start,
      annotated_block_component.options);
  const visage::Screenshot block_loop_1 = adt::canonical::renderCanonicalGraphicFrame(
      annotated_block_component.canonical_id,
      annotated_block_component.dimensions,
      block_loop_end,
      annotated_block_component.options);

  if (!screenshotsNearlyMatch(block_loop_0, block_loop_1)) {
    std::cerr << "Canonical block processing loop endpoint does not wrap cleanly.\n";
    return 1;
  }

  return 0;
}
