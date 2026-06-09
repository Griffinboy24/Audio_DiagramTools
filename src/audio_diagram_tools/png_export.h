#pragma once

#include <string>

namespace visage {
class Screenshot;
}

namespace adt {

void savePngWithStraightAlpha(const std::string& output_path, const visage::Screenshot& screenshot);

} // namespace adt
