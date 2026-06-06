#include "audio_diagram_tools/gif_exporter.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct CliOptions {
  adt::GifExportSpec export_spec;
  adt::WaveformSpec waveform;
  std::filesystem::path output_path = "artifacts/gifs/am_sine.gif";
  bool help = false;
  bool list_presets = false;
};

void printHelp() {
  std::cout
    << "Audio Diagram Tools GIF encoder\n\n"
    << "Usage:\n"
    << "  adt_encode_gif --out artifacts/gifs/am_sine.gif --frames 120 --fps 30\n\n"
    << "Options:\n"
    << "  --preset <id>            Use a named canvas preset.\n"
    << "  --list-presets           List available canvas presets.\n"
    << "  --width <px>              Output width. Default: 1280\n"
    << "  --height <px>             Output height. Default: 720\n"
    << "  --fps <value>             Animation frame rate. Default: 30\n"
    << "  --frames <count>          Number of frames to render. Default: 120\n"
    << "  --out <gif>               GIF output path.\n"
    << "  --carrier-cycles <value>  Carrier cycles across the plot. Default: 7\n"
    << "  --mod-cycles <value>      Modulator cycles across the plot. Default: 1\n"
    << "  --mod-depth <0..1>        Volume modulation depth. Default: 0.72\n"
    << "  --no-dither               Disable cgif RGB dithering.\n"
    << "  --no-grid                 Hide grid.\n"
    << "  --no-envelope             Hide modulation envelope.\n"
    << "  --help                    Show this help.\n";
}

void printPresets() {
  std::cout << "Canvas presets:\n";
  for (const auto& preset : adt::canvasPresets()) {
    std::cout << "  " << preset.id << "  " << preset.dimensions.width << "x"
              << preset.dimensions.height << "  " << preset.description << "\n";
  }
}

bool nextValue(const std::vector<std::string>& args, size_t* index, std::string* value) {
  if (*index + 1 >= args.size())
    return false;

  *value = args[++(*index)];
  return true;
}

int parseInt(const std::string& value, const std::string& option) {
  try {
    return std::stoi(value);
  }
  catch (const std::exception&) {
    throw std::runtime_error("Invalid integer for " + option + ": " + value);
  }
}

double parseDouble(const std::string& value, const std::string& option) {
  try {
    return std::stod(value);
  }
  catch (const std::exception&) {
    throw std::runtime_error("Invalid number for " + option + ": " + value);
  }
}

CliOptions parseArgs(int argc, char** argv) {
  CliOptions options;
  std::vector<std::string> args(argv + 1, argv + argc);

  for (size_t i = 0; i < args.size(); ++i) {
    const std::string& arg = args[i];
    std::string value;

    if (arg == "--help" || arg == "-h") {
      options.help = true;
    }
    else if (arg == "--list-presets") {
      options.list_presets = true;
    }
    else if (arg == "--preset" && nextValue(args, &i, &value)) {
      options.export_spec.dimensions = adt::dimensionsForPreset(value);
    }
    else if (arg == "--width" && nextValue(args, &i, &value)) {
      options.export_spec.dimensions.width = parseInt(value, arg);
    }
    else if (arg == "--height" && nextValue(args, &i, &value)) {
      options.export_spec.dimensions.height = parseInt(value, arg);
    }
    else if (arg == "--fps" && nextValue(args, &i, &value)) {
      options.export_spec.fps = parseDouble(value, arg);
    }
    else if (arg == "--frames" && nextValue(args, &i, &value)) {
      options.export_spec.frame_count = parseInt(value, arg);
    }
    else if (arg == "--out" && nextValue(args, &i, &value)) {
      options.output_path = value;
    }
    else if (arg == "--carrier-cycles" && nextValue(args, &i, &value)) {
      options.waveform.carrier_cycles = static_cast<float>(parseDouble(value, arg));
    }
    else if (arg == "--mod-cycles" && nextValue(args, &i, &value)) {
      options.waveform.modulation_cycles = static_cast<float>(parseDouble(value, arg));
    }
    else if (arg == "--mod-depth" && nextValue(args, &i, &value)) {
      options.waveform.modulation_depth = static_cast<float>(parseDouble(value, arg));
    }
    else if (arg == "--no-dither") {
      options.export_spec.dither = false;
    }
    else if (arg == "--no-grid") {
      options.waveform.show_grid = false;
    }
    else if (arg == "--no-envelope") {
      options.waveform.show_envelope = false;
    }
    else {
      throw std::runtime_error("Unknown or incomplete option: " + arg);
    }
  }

  if (options.export_spec.dimensions.width <= 0 || options.export_spec.dimensions.height <= 0)
    throw std::runtime_error("Width and height must be positive.");
  if (options.export_spec.frame_count <= 0)
    throw std::runtime_error("Frame count must be positive.");
  if (options.export_spec.fps <= 0.0)
    throw std::runtime_error("FPS must be positive.");

  return options;
}

void ensureParentDirectory(const std::filesystem::path& file_path) {
  const auto parent = file_path.parent_path();
  if (!parent.empty())
    std::filesystem::create_directories(parent);
}

} // namespace

int main(int argc, char** argv) {
  try {
    const CliOptions options = parseArgs(argc, argv);
    if (options.help) {
      printHelp();
      return 0;
    }
    if (options.list_presets) {
      printPresets();
      return 0;
    }

    ensureParentDirectory(options.output_path);
    adt::saveVolumeModulatedSineGif(options.output_path.string(), options.export_spec, {},
                                    options.waveform);

    std::cout << "Rendered " << options.export_spec.frame_count << " frames to "
              << options.output_path.string() << "\n";
  }
  catch (const std::exception& e) {
    std::cerr << "adt_encode_gif: " << e.what() << "\n\n";
    printHelp();
    return 1;
  }

  return 0;
}
