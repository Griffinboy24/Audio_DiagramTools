#include "audio_diagram_tools/png_export.h"

#include <algorithm>
#include <cstdint>

#include <visage/graphics.h>

namespace adt {
namespace {

uint8_t normalizeChannel(uint8_t channel, uint8_t alpha) {
  if (alpha == 0 || alpha == 255)
    return channel;

  const uint64_t alpha_squared = static_cast<uint64_t>(alpha) * static_cast<uint64_t>(alpha);
  const uint64_t numerator = static_cast<uint64_t>(channel) * 255u * 255u + alpha_squared / 2u;
  const uint64_t value = numerator / alpha_squared;
  return static_cast<uint8_t>(std::min<uint64_t>(255u, value));
}

void normalizeStraightAlpha(visage::Screenshot& screenshot) {
  uint8_t* data = screenshot.data();
  const int pixel_count = screenshot.width() * screenshot.height();

  for (int pixel = 0; pixel < pixel_count; ++pixel) {
    uint8_t* rgba = data + pixel * 4;
    const uint8_t alpha = rgba[3];
    if (alpha <= 3) {
      rgba[0] = 0;
      rgba[1] = 0;
      rgba[2] = 0;
      rgba[3] = 0;
      continue;
    }

    rgba[0] = normalizeChannel(rgba[0], alpha);
    rgba[1] = normalizeChannel(rgba[1], alpha);
    rgba[2] = normalizeChannel(rgba[2], alpha);
  }
}

} // namespace

void savePngWithStraightAlpha(const std::string& output_path, const visage::Screenshot& screenshot) {
  visage::Screenshot normalized = screenshot;
  normalizeStraightAlpha(normalized);
  normalized.save(output_path);
}

} // namespace adt
