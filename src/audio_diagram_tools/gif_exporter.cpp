#include "audio_diagram_tools/gif_exporter.h"

#include "audio_diagram_tools/waveform_scene.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <cgif.h>

namespace adt {
namespace {

std::vector<uint8_t> screenshotToRgb(const visage::Screenshot& screenshot) {
  const int pixel_count = screenshot.width() * screenshot.height();
  const uint8_t* rgba = screenshot.data();
  std::vector<uint8_t> rgb(static_cast<size_t>(pixel_count) * 3);

  for (int pixel = 0; pixel < pixel_count; ++pixel) {
    rgb[static_cast<size_t>(pixel) * 3] = rgba[static_cast<size_t>(pixel) * 4];
    rgb[static_cast<size_t>(pixel) * 3 + 1] = rgba[static_cast<size_t>(pixel) * 4 + 1];
    rgb[static_cast<size_t>(pixel) * 3 + 2] = rgba[static_cast<size_t>(pixel) * 4 + 2];
  }

  return rgb;
}

uint16_t delayHundredths(double fps) {
  const double safe_fps = fps > 0.0 ? fps : 30.0;
  return static_cast<uint16_t>(std::max(1.0, std::round(100.0 / safe_fps)));
}

void checkResult(cgif_result result, const std::string& action) {
  if (result != CGIF_OK)
    throw std::runtime_error(action + " failed with cgif error " + std::to_string(static_cast<int>(result)));
}

} // namespace

void saveVolumeModulatedSineGif(const std::string& output_path,
                                const GifExportSpec& export_spec,
                                const RenderStyle& style,
                                const WaveformSpec& waveform) {
  if (export_spec.dimensions.width <= 0 || export_spec.dimensions.height <= 0)
    throw std::runtime_error("GIF dimensions must be positive.");
  if (export_spec.dimensions.width > 65535 || export_spec.dimensions.height > 65535)
    throw std::runtime_error("GIF dimensions exceed the format limit.");
  if (export_spec.frame_count <= 0)
    throw std::runtime_error("GIF frame count must be positive.");
  if (export_spec.fps <= 0.0)
    throw std::runtime_error("GIF FPS must be positive.");

  CGIFrgb_Config config = {};
  config.path = output_path.c_str();
  config.width = static_cast<uint16_t>(export_spec.dimensions.width);
  config.height = static_cast<uint16_t>(export_spec.dimensions.height);
  config.numLoops = CGIF_INFINITE_LOOP;

  CGIFrgb* gif = cgif_rgb_newgif(&config);
  if (!gif)
    throw std::runtime_error("Could not create GIF: " + output_path);

  const uint16_t delay = delayHundredths(export_spec.fps);
  bool closed = false;

  try {
    for (int frame = 0; frame < export_spec.frame_count; ++frame) {
      const Timeline timeline = Timeline::forFrame(frame, export_spec.frame_count, export_spec.fps);
      visage::Screenshot screenshot =
        renderVolumeModulatedSineFrame(export_spec.dimensions, timeline, style, waveform);
      std::vector<uint8_t> rgb = screenshotToRgb(screenshot);

      CGIFrgb_FrameConfig frame_config = {};
      frame_config.pImageData = rgb.data();
      frame_config.fmtChan = CGIF_CHAN_FMT_RGB;
      frame_config.delay = delay;
      if (!export_spec.dither)
        frame_config.attrFlags |= CGIF_RGB_FRAME_ATTR_NO_DITHERING;

      checkResult(cgif_rgb_addframe(gif, &frame_config), "Adding GIF frame");
    }

    const cgif_result close_result = cgif_rgb_close(gif);
    closed = true;
    checkResult(close_result, "Closing GIF");
  }
  catch (...) {
    if (!closed)
      cgif_rgb_close(gif);
    throw;
  }
}

} // namespace adt
