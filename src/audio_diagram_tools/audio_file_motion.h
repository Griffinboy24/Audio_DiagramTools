#pragma once

#include "audio_diagram_tools/render_types.h"

namespace adt {

struct AudioFileSweep {
  float playhead_progress = 0.0f;
  bool erase_pass = false;
  bool show_playhead = true;
  bool show_waveform = true;
};

float audioFileWaveformValue(float progress);
float voiceSampleWaveformValue(float progress);
float audioFilePlayheadProgress(const Timeline& timeline);
AudioFileSweep audioFileTwoStageSweep(const Timeline& timeline);
AudioFileSweep voiceSampleOneShotSweep(const Timeline& timeline);
float speakerDriveForAudioFileProgress(float progress, float gain = 1.18f);
float speakerDriveForVoiceSampleProgress(float progress, float gain = 1.35f);

} // namespace adt
