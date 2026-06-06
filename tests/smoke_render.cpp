#include "audio_diagram_tools/waveform_scene.h"

#include <cstdint>
#include <iostream>

int main() {
  const adt::Dimensions dimensions { 320, 180 };
  const adt::Timeline timeline = adt::Timeline::forFrame(3, 30, 30.0);
  const visage::Screenshot screenshot = adt::renderVolumeModulatedSineFrame(dimensions, timeline);

  if (screenshot.width() != dimensions.width || screenshot.height() != dimensions.height) {
    std::cerr << "Unexpected screenshot dimensions: " << screenshot.width() << "x"
              << screenshot.height() << "\n";
    return 1;
  }

  const uint8_t* data = screenshot.data();
  const int bytes = dimensions.width * dimensions.height * 4;
  int changed_samples = 0;

  for (int i = 4; i < bytes; i += 97) {
    if (data[i] != data[0] || data[i + 1] != data[1] || data[i + 2] != data[2])
      ++changed_samples;
  }

  if (changed_samples < 64) {
    std::cerr << "Rendered image appears mostly blank.\n";
    return 1;
  }

  return 0;
}
