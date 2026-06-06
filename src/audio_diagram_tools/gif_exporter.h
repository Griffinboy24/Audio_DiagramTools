#pragma once

#include "audio_diagram_tools/render_types.h"

#include <functional>
#include <string>
#include <visage/graphics.h>

namespace adt {

struct GifExportSpec {
  Dimensions dimensions;
  int frame_count = 120;
  double fps = 30.0;
  bool dither = true;
};

using FrameRenderer = std::function<visage::Screenshot(const Timeline&)>;

void saveAnimatedGif(const std::string& output_path,
                     const GifExportSpec& export_spec,
                     const FrameRenderer& render_frame);

void saveVolumeModulatedSineGif(const std::string& output_path,
                                const GifExportSpec& export_spec,
                                const RenderStyle& style = {},
                                const WaveformSpec& waveform = {});

} // namespace adt
