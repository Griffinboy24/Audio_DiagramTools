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

  const auto playback_component = adt::canonical::sampleTablePlaybackScene();
  const auto playback_metadata =
      adt::canonical::canonicalGraphicById(playback_component.canonical_id);
  if (!playback_metadata) {
    std::cerr << "Canonical sample table playback scene lookup failed.\n";
    return 1;
  }

  const visage::Screenshot playback_scene = adt::canonical::renderCanonicalGraphicFrame(
      playback_component.canonical_id,
      playback_component.dimensions,
      timeline,
      playback_component.options);

  if (playback_scene.width() != playback_component.dimensions.width ||
      playback_scene.height() != playback_component.dimensions.height ||
      !hasVisibleVariation(playback_scene)) {
    std::cerr << "Canonical sample table playback scene render failed.\n";
    return 1;
  }

  const auto table_waveform_component = adt::canonical::sampleTableWaveformGraphic();
  const auto table_waveform_metadata =
      adt::canonical::canonicalGraphicById(table_waveform_component.canonical_id);
  if (!table_waveform_metadata) {
    std::cerr << "Canonical sample table waveform graphic lookup failed.\n";
    return 1;
  }

  const visage::Screenshot table_waveform = adt::canonical::renderCanonicalGraphicFrame(
      table_waveform_component.canonical_id,
      table_waveform_component.dimensions,
      timeline,
      table_waveform_component.options);

  if (table_waveform.width() != table_waveform_component.dimensions.width ||
      table_waveform.height() != table_waveform_component.dimensions.height ||
      !hasVisibleVariation(table_waveform)) {
    std::cerr << "Canonical sample table waveform graphic render failed.\n";
    return 1;
  }

  const auto dense_component = adt::canonical::denseSampleWaveformGraphic();
  const auto dense_metadata =
      adt::canonical::canonicalGraphicById(dense_component.canonical_id);
  if (!dense_metadata) {
    std::cerr << "Canonical dense sample waveform graphic lookup failed.\n";
    return 1;
  }

  const visage::Screenshot dense_waveform = adt::canonical::renderCanonicalGraphicFrame(
      dense_component.canonical_id,
      dense_component.dimensions,
      timeline,
      dense_component.options);

  if (dense_waveform.width() != dense_component.dimensions.width ||
      dense_waveform.height() != dense_component.dimensions.height ||
      !hasVisibleVariation(dense_waveform)) {
    std::cerr << "Canonical dense sample waveform graphic render failed.\n";
    return 1;
  }

  const auto buffer_split_component = adt::canonical::waveformBufferSplitGraphic();
  const auto buffer_split_metadata =
      adt::canonical::canonicalGraphicById(buffer_split_component.canonical_id);
  if (!buffer_split_metadata) {
    std::cerr << "Canonical waveform buffer split graphic lookup failed.\n";
    return 1;
  }

  const visage::Screenshot buffer_split = adt::canonical::renderCanonicalGraphicFrame(
      buffer_split_component.canonical_id,
      buffer_split_component.dimensions,
      timeline,
      buffer_split_component.options);

  if (buffer_split.width() != buffer_split_component.dimensions.width ||
      buffer_split.height() != buffer_split_component.dimensions.height ||
      !hasVisibleVariation(buffer_split)) {
    std::cerr << "Canonical waveform buffer split graphic render failed.\n";
    return 1;
  }

  const auto volume_component = adt::canonical::waveformVolumeScaleGraphic();
  const auto volume_metadata =
      adt::canonical::canonicalGraphicById(volume_component.canonical_id);
  if (!volume_metadata) {
    std::cerr << "Canonical waveform volume scale graphic lookup failed.\n";
    return 1;
  }

  const visage::Screenshot volume_scene = adt::canonical::renderCanonicalGraphicFrame(
      volume_component.canonical_id,
      volume_component.dimensions,
      timeline,
      volume_component.options);

  if (volume_scene.width() != volume_component.dimensions.width ||
      volume_scene.height() != volume_component.dimensions.height ||
      !hasVisibleVariation(volume_scene)) {
    std::cerr << "Canonical waveform volume scale graphic render failed.\n";
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

  const auto oscillator_component =
      adt::canonical::oscillatorBlockFactoryGraphic({}, { 920, 330 });
  const auto oscillator_metadata =
      adt::canonical::canonicalGraphicById(oscillator_component.canonical_id);
  if (!oscillator_metadata) {
    std::cerr << "Canonical oscillator block factory lookup failed.\n";
    return 1;
  }

  const visage::Screenshot oscillator = adt::canonical::renderCanonicalGraphicFrame(
      oscillator_component.canonical_id,
      oscillator_component.dimensions,
      timeline,
      oscillator_component.options);

  if (oscillator.width() != 920 || oscillator.height() != 330 ||
      !hasVisibleVariation(oscillator)) {
    std::cerr << "Canonical oscillator block factory render failed.\n";
    return 1;
  }

  const visage::Screenshot oscillator_loop_0 = adt::canonical::renderCanonicalGraphicFrame(
      oscillator_component.canonical_id,
      oscillator_component.dimensions,
      block_loop_start,
      oscillator_component.options);
  const visage::Screenshot oscillator_loop_1 = adt::canonical::renderCanonicalGraphicFrame(
      oscillator_component.canonical_id,
      oscillator_component.dimensions,
      block_loop_end,
      oscillator_component.options);

  if (!screenshotsNearlyMatch(oscillator_loop_0, oscillator_loop_1)) {
    std::cerr << "Canonical oscillator block factory loop endpoint does not wrap cleanly.\n";
    return 1;
  }

  const auto plugin_chain_component =
      adt::canonical::pluginChainRoutingGraphic({}, { 920, 520 });
  const auto plugin_chain_metadata =
      adt::canonical::canonicalGraphicById(plugin_chain_component.canonical_id);
  if (!plugin_chain_metadata) {
    std::cerr << "Canonical plugin chain routing lookup failed.\n";
    return 1;
  }

  const visage::Screenshot plugin_chain = adt::canonical::renderCanonicalGraphicFrame(
      plugin_chain_component.canonical_id,
      plugin_chain_component.dimensions,
      timeline,
      plugin_chain_component.options);

  if (plugin_chain.width() != 920 || plugin_chain.height() != 520 ||
      !hasVisibleVariation(plugin_chain)) {
    std::cerr << "Canonical plugin chain routing render failed.\n";
    return 1;
  }

  const visage::Screenshot plugin_chain_loop_0 = adt::canonical::renderCanonicalGraphicFrame(
      plugin_chain_component.canonical_id,
      plugin_chain_component.dimensions,
      block_loop_start,
      plugin_chain_component.options);
  const visage::Screenshot plugin_chain_loop_1 = adt::canonical::renderCanonicalGraphicFrame(
      plugin_chain_component.canonical_id,
      plugin_chain_component.dimensions,
      block_loop_end,
      plugin_chain_component.options);

  if (!screenshotsNearlyMatch(plugin_chain_loop_0, plugin_chain_loop_1)) {
    std::cerr << "Canonical plugin chain routing loop endpoint does not wrap cleanly.\n";
    return 1;
  }

  const auto output_speaker_component =
      adt::canonical::outputStreamToSpeakerGraphic({}, { 920, 300 });
  const auto output_speaker_metadata =
      adt::canonical::canonicalGraphicById(output_speaker_component.canonical_id);
  if (!output_speaker_metadata) {
    std::cerr << "Canonical output stream to speaker lookup failed.\n";
    return 1;
  }

  const visage::Screenshot output_speaker = adt::canonical::renderCanonicalGraphicFrame(
      output_speaker_component.canonical_id,
      output_speaker_component.dimensions,
      timeline,
      output_speaker_component.options);

  if (output_speaker.width() != 920 || output_speaker.height() != 300 ||
      !hasVisibleVariation(output_speaker)) {
    std::cerr << "Canonical output stream to speaker render failed.\n";
    return 1;
  }

  const visage::Screenshot output_speaker_loop_0 = adt::canonical::renderCanonicalGraphicFrame(
      output_speaker_component.canonical_id,
      output_speaker_component.dimensions,
      block_loop_start,
      output_speaker_component.options);
  const visage::Screenshot output_speaker_loop_1 = adt::canonical::renderCanonicalGraphicFrame(
      output_speaker_component.canonical_id,
      output_speaker_component.dimensions,
      block_loop_end,
      output_speaker_component.options);

  if (!screenshotsNearlyMatch(output_speaker_loop_0, output_speaker_loop_1)) {
    std::cerr << "Canonical output stream to speaker loop endpoint does not wrap cleanly.\n";
    return 1;
  }

  return 0;
}
