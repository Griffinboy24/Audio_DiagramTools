#pragma once

#include "audio_diagram_tools/render_types.h"

#include <string>

namespace adt {

struct GifExportSpec {
  Dimensions dimensions;
  int frame_count = 120;
  double fps = 30.0;
  bool dither = true;
};

void saveVolumeModulatedSineGif(const std::string& output_path,
                                const GifExportSpec& export_spec,
                                const RenderStyle& style = {},
                                const WaveformSpec& waveform = {});

} // namespace adt
