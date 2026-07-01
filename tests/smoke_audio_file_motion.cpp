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

  if (!near(adt::voiceSampleWaveformValue(0.0f), 0.0f, 0.0001f) ||
      !near(adt::voiceSampleWaveformValue(1.0f), 0.0f, 0.0001f)) {
    std::cerr << "Voice sample waveform should start and end in silence.\n";
    return 1;
  }

  const adt::AudioFileSweep voice_playing =
      adt::voiceSampleOneShotSweep(adt::Timeline::forFrame(12, 100, 20.0));
  if (!voice_playing.show_playhead || voice_playing.playhead_progress <= 0.0f ||
      voice_playing.playhead_progress >= 1.0f || voice_playing.erase_pass ||
      !voice_playing.show_waveform) {
    std::cerr << "Voice sample first pass is not mapped into the sample body.\n";
    return 1;
  }

  const adt::AudioFileSweep voice_midpoint_pause =
      adt::voiceSampleOneShotSweep(adt::Timeline::forFrame(34, 100, 20.0));
  if (voice_midpoint_pause.show_playhead || !voice_midpoint_pause.show_waveform ||
      voice_midpoint_pause.erase_pass ||
      voice_midpoint_pause.playhead_progress < 0.99f ||
      voice_midpoint_pause.playhead_progress >= 1.0f) {
    std::cerr << "Voice sample should hide the playhead while holding before the wipe pass.\n";
    return 1;
  }

  const adt::AudioFileSweep voice_second_pass =
      adt::voiceSampleOneShotSweep(adt::Timeline::forFrame(62, 100, 20.0));
  if (!voice_second_pass.show_playhead || !voice_second_pass.show_waveform ||
      !voice_second_pass.erase_pass || voice_second_pass.playhead_progress <= 0.0f ||
      voice_second_pass.playhead_progress >= 1.0f) {
    std::cerr << "Voice sample second pass should match the old file-player sweep.\n";
    return 1;
  }

  const adt::AudioFileSweep voice_end_pause =
      adt::voiceSampleOneShotSweep(adt::Timeline::forFrame(96, 100, 20.0));
  if (voice_end_pause.show_playhead || !voice_end_pause.show_waveform ||
      !voice_end_pause.erase_pass || voice_end_pause.playhead_progress < 0.99f ||
      voice_end_pause.playhead_progress >= 1.0f) {
    std::cerr << "Voice sample should hide the playhead while holding after the wipe pass.\n";
    return 1;
  }

  for (int frame : { 1, 25, 45, 70, 96 }) {
    const adt::AudioFileSweep sweep =
        adt::voiceSampleOneShotSweep(adt::Timeline::forFrame(frame, 100, 20.0));
    if (!sweep.show_waveform) {
      std::cerr << "Voice sample should keep the old player visible throughout the loop.\n";
      return 1;
    }
  }

  return 0;
}
