#include "audio_diagram_tools/audio_file_motion.h"

#include <cmath>
#include <iostream>

namespace {

bool near(float a, float b, float tolerance) {
  return std::abs(a - b) <= tolerance;
}

} // namespace

int main() {
  const float start = adt::audioFileWaveformValue(0.0f);
  const float end = adt::audioFileWaveformValue(1.0f);
  const float before_wrap = adt::audioFileWaveformValue(0.999f);

  if (!near(start, end, 0.0001f)) {
    std::cerr << "Audio waveform loop endpoints are discontinuous.\n";
    return 1;
  }

  if (std::abs(before_wrap - start) > 0.08f) {
    std::cerr << "Audio waveform approaches loop endpoint too abruptly.\n";
    return 1;
  }

  const adt::AudioFileSweep first_half =
      adt::audioFileTwoStageSweep(adt::Timeline::forFrame(22, 100, 20.0));
  if (first_half.erase_pass || !near(first_half.playhead_progress, 0.44f, 0.0001f)) {
    std::cerr << "Unexpected first-half sweep mapping.\n";
    return 1;
  }

  const adt::AudioFileSweep second_half =
      adt::audioFileTwoStageSweep(adt::Timeline::forFrame(72, 100, 20.0));
  if (!second_half.erase_pass || !near(second_half.playhead_progress, 0.44f, 0.0001f)) {
    std::cerr << "Unexpected erase-pass sweep mapping.\n";
    return 1;
  }

  const float driven = adt::speakerDriveForAudioFileProgress(0.44f);
  if (driven < -1.0f || driven > 1.0f) {
    std::cerr << "Speaker drive escaped normalized range.\n";
    return 1;
  }

  return 0;
}
