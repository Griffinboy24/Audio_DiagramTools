#pragma once

#include "audio_diagram_tools/render_types.h"

#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <visage/graphics.h>

namespace adt {

struct StyleStudy {
  std::string_view id;
  std::string_view description;
};

const std::array<StyleStudy, 23>& styleStudies();
std::optional<StyleStudy> styleStudyById(std::string_view id);

visage::Screenshot renderStyleStudyFrame(std::string_view study_id,
                                         const Dimensions& dimensions,
                                         const Timeline& timeline = {});

void saveStyleStudyFrame(const std::string& output_path,
                         std::string_view study_id,
                         const Dimensions& dimensions,
                         const Timeline& timeline = {});

} // namespace adt
