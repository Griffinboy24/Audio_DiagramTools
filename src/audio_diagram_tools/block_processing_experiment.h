#pragma once

#include "audio_diagram_tools/render_types.h"

#include <string>

#include <visage/graphics.h>

namespace adt::experiments {

visage::Screenshot renderBlockProcessingExperimentFrame(const Dimensions& dimensions,
                                                        const Timeline& timeline);

void saveBlockProcessingExperimentFrame(const std::string& output_path,
                                        const Dimensions& dimensions,
                                        const Timeline& timeline = {});

} // namespace adt::experiments
