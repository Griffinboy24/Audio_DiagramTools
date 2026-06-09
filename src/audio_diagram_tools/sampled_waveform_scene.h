#pragma once

#include "audio_diagram_tools/render_types.h"

#include <array>
#include <string>
#include <visage/graphics.h>

namespace adt {

struct EightSampleWaveformSpec {
  std::array<float, 8> amplitudes = { 0.0f, 0.42f, 0.88f, 0.56f,
                                      -0.12f, -0.76f, -0.48f, 0.22f };
};

visage::Screenshot renderEightSampleWaveformFrame(const Dimensions& dimensions,
                                                  const EightSampleWaveformSpec& spec = {});

void saveEightSampleWaveformFrame(const std::string& output_path,
                                  const Dimensions& dimensions,
                                  const EightSampleWaveformSpec& spec = {});

} // namespace adt
