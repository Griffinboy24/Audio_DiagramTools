#include "audio_diagram_tools/composition.h"
#include "audio_diagram_tools/composite_graphics.h"

#include <cstdint>
#include <iostream>

namespace {

bool hasVisibleVariation(const visage::Screenshot& screenshot) {
  const uint8_t* data = screenshot.data();
  const int bytes = screenshot.width() * screenshot.height() * 4;
  int changed_samples = 0;

  for (int i = 4; i < bytes; i += 97) {
    if (data[i] != data[0] || data[i + 1] != data[1] || data[i + 2] != data[2])
      ++changed_samples;
  }

  return changed_samples > 64;
}

} // namespace

int main() {
  adt::canonical::AudioFilePlayerOptions player_options;
  player_options.clear_background = false;
  player_options.playhead_progress = 0.42f;

  adt::canonical::DoubleArrowOptions arrow_options;
  arrow_options.single_color = 0xff4e4e4e;

  adt::canonical::SpeakerMotionOptions speaker_options;
  speaker_options.clear_background = false;
  speaker_options.draw_caption = false;
  speaker_options.cone_drive = 0.35f;
  speaker_options.sound_drive = 0.35f;

  adt::composition::StackBuilder builder(adt::composition::hiseInlineProfile(),
                                         { 56.0f, 24.0f, 58.0f, false });
  builder.add(adt::canonical::audioFilePlayerGraphic(player_options, { 565, 159 }), 34.0f)
      .add(adt::canonical::doubleArrowGraphic(arrow_options, { 46, 36 }), 24.0f)
      .add(adt::canonical::speakerAnimationGraphic(speaker_options, { 390, 226 }), 22.0f);

  adt::composition::Scene scene = builder.build();
  adt::composition::addCenteredLabelBodyLine(
      scene, "Top:", "Sound over time (waveform).", 552.0f, 380.0f);

  const adt::Timeline timeline = adt::Timeline::forFrame(8, 90, 18.0);
  const visage::Screenshot screenshot = adt::composition::renderSceneFrame(scene, timeline);

  if (screenshot.width() != scene.profile.dimensions.width ||
      screenshot.height() != scene.profile.dimensions.height) {
    std::cerr << "Unexpected composition dimensions.\n";
    return 1;
  }

  if (!hasVisibleVariation(screenshot)) {
    std::cerr << "Composition render appears blank.\n";
    return 1;
  }

  adt::canonical::BlockProcessingOptions block_options;
  block_options.clear_background = true;

  adt::composition::Profile dark_profile = adt::composition::hiseDarkArticleImageProfile(368);
  adt::composition::StackOptions block_stack_options;
  block_stack_options.top_padding = dark_profile.top_padding;
  block_stack_options.default_gap = 0.0f;
  block_stack_options.bottom_padding = dark_profile.bottom_padding;
  block_stack_options.auto_height = false;

  adt::composition::StackBuilder block_builder(dark_profile, block_stack_options);
  block_builder.add(adt::canonical::blockProcessingGraphic(block_options, { 920, 330 }), 0.0f);
  const adt::composition::Scene block_scene = block_builder.build();
  const visage::Screenshot block_screenshot =
      adt::composition::renderSceneFrame(block_scene, timeline);

  if (block_screenshot.width() != 920 || block_screenshot.height() != 368 ||
      !hasVisibleVariation(block_screenshot)) {
    std::cerr << "Block processing composition render failed.\n";
    return 1;
  }

  const adt::composition::Scene gain_scene =
      adt::composites::sampleGainComparisonScene();
  const visage::Screenshot gain_screenshot =
      adt::composition::renderSceneFrame(gain_scene, timeline);

  if (gain_screenshot.width() != 920 || gain_screenshot.height() != 700 ||
      !hasVisibleVariation(gain_screenshot)) {
    std::cerr << "Sample gain comparison composition render failed.\n";
    return 1;
  }

  const adt::composition::Scene playback_scene =
      adt::composites::sampleTablePlaybackArticleScene();
  const visage::Screenshot playback_screenshot =
      adt::composition::renderSceneFrame(playback_scene, timeline);

  if (playback_screenshot.width() != 920 || playback_screenshot.height() != 430 ||
      !hasVisibleVariation(playback_screenshot)) {
    std::cerr << "Sample table playback article composition render failed.\n";
      return 1;
  }

  const adt::composition::Scene array_to_plot_scene =
      adt::composites::sampleArrayToPlotArticleScene();
  const visage::Screenshot array_to_plot_screenshot =
      adt::composition::renderSceneFrame(array_to_plot_scene, timeline);

  if (array_to_plot_screenshot.width() != 920 || array_to_plot_screenshot.height() != 520 ||
      !hasVisibleVariation(array_to_plot_screenshot)) {
    std::cerr << "Sample array-to-plot article composition render failed.\n";
    return 1;
  }

  const adt::composition::Scene plugin_chain_scene =
      adt::composites::pluginChainRoutingArticleScene();
  const visage::Screenshot plugin_chain_screenshot =
      adt::composition::renderSceneFrame(plugin_chain_scene, timeline);

  if (plugin_chain_screenshot.width() != 920 || plugin_chain_screenshot.height() != 560 ||
      !hasVisibleVariation(plugin_chain_screenshot)) {
    std::cerr << "Plugin chain routing article composition render failed.\n";
    return 1;
  }

  const adt::composition::Scene output_speaker_scene =
      adt::composites::outputStreamToSpeakerArticleScene();
  const visage::Screenshot output_speaker_screenshot =
      adt::composition::renderSceneFrame(output_speaker_scene, timeline);

  if (output_speaker_screenshot.width() != 920 ||
      output_speaker_screenshot.height() != 340 ||
      !hasVisibleVariation(output_speaker_screenshot)) {
    std::cerr << "Output stream to speaker article composition render failed.\n";
    return 1;
  }

  return 0;
}
