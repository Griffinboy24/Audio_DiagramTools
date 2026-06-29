#include "audio_diagram_tools/composition.h"

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

  return 0;
}
