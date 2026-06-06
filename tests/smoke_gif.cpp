#include "audio_diagram_tools/gif_exporter.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

int main() {
  const std::filesystem::path output =
    std::filesystem::temp_directory_path() / "audio_diagram_tools_smoke.gif";

  adt::GifExportSpec spec;
  spec.dimensions = { 160, 90 };
  spec.frame_count = 4;
  spec.fps = 12.0;

  try {
    adt::saveVolumeModulatedSineGif(output.string(), spec);
  }
  catch (const std::exception& e) {
    std::cerr << "GIF export threw: " << e.what() << "\n";
    return 1;
  }

  char header[6] = {};
  {
    std::ifstream file(output, std::ios::binary);
    file.read(header, sizeof(header));
  }

  const bool has_gif_header =
    std::string(header, sizeof(header)) == "GIF89a" ||
    std::string(header, sizeof(header)) == "GIF87a";

  const auto file_size = std::filesystem::file_size(output);
  std::filesystem::remove(output);

  if (!has_gif_header || file_size < 256) {
    std::cerr << "GIF export did not produce a plausible GIF.\n";
    return 1;
  }

  return 0;
}
