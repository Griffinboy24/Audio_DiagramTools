#pragma once

#include "audio_diagram_tools/render_types.h"

namespace adt {

struct AudioFileSweep {
  float playhead_progress = 0.0f;
  bool erase_pass = false;
};

float audioFileWaveformValue(float progress);
float audioFilePlayheadProgress(const Timeline& timeline);
AudioFileSweep audioFileTwoStageSweep(const Timeline& timeline);
float speakerDriveForAudioFileProgress(float progress, float gain = 1.18f);

} // namespace adt
