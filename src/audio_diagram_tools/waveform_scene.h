#pragma once

#include "audio_diagram_tools/render_types.h"

#include <string>
#include <visage/graphics.h>

namespace adt {

void drawVolumeModulatedSine(visage::Canvas& canvas,
                             const Dimensions& dimensions,
                             const Timeline& timeline,
                             const RenderStyle& style = {},
                             const WaveformSpec& waveform = {});

visage::Screenshot renderVolumeModulatedSineFrame(const Dimensions& dimensions,
                                                  const Timeline& timeline,
                                                  const RenderStyle& style = {},
                                                  const WaveformSpec& waveform = {});

void saveVolumeModulatedSineFrame(const std::string& output_path,
                                  const Dimensions& dimensions,
                                  const Timeline& timeline,
                                  const RenderStyle& style = {},
                                  const WaveformSpec& waveform = {});

} // namespace adt
