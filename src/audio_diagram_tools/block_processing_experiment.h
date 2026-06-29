#pragma once

#include "audio_diagram_tools/canonical_components.h"
#include "audio_diagram_tools/canonical_drawing.h"
#include "audio_diagram_tools/render_types.h"

#include <string>

#include <visage/graphics.h>

namespace adt::experiments {

void drawBlockProcessingExperiment(canonical::drawing::DrawContext& context,
                                   const Dimensions& dimensions,
                                   const Timeline& timeline,
                                   const canonical::BlockProcessingOptions& options = {});

visage::Screenshot renderBlockProcessingExperimentFrame(const Dimensions& dimensions,
                                                        const Timeline& timeline);

void saveBlockProcessingExperimentFrame(const std::string& output_path,
                                        const Dimensions& dimensions,
                                        const Timeline& timeline = {});

} // namespace adt::experiments
