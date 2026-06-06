#include "audio_diagram_tools/waveform_scene.h"

#include <filesystem>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct CliOptions {
  adt::Dimensions dimensions;
  adt::WaveformSpec waveform;
  int frames = 1;
  double fps = 30.0;
  std::filesystem::path output_path = "artifacts/stills/am_sine.png";
  std::filesystem::path output_dir = "artifacts/frames/am_sine";
  bool sequence = false;
  bool help = false;
  bool list_presets = false;
};

void printHelp() {
  std::cout
    << "Audio Diagram Tools renderer\n\n"
    << "Usage:\n"
    << "  adt_render --out artifacts/stills/am_sine.png\n"
    << "  adt_render --frames 120 --out-dir artifacts/frames/am_sine --fps 30\n\n"
    << "Options:\n"
    << "  --preset <id>            Use a named canvas preset.\n"
    << "  --list-presets           List available canvas presets.\n"
    << "  --width <px>              Output width. Default: 1280\n"
    << "  --height <px>             Output height. Default: 720\n"
    << "  --fps <value>             Animation frame rate. Default: 30\n"
    << "  --frames <count>          Number of frames to render. Default: 1\n"
    << "  --out <png>               Still image output path.\n"
    << "  --out-dir <directory>     Frame sequence directory.\n"
    << "  --carrier-cycles <value>  Carrier cycles across the plot. Default: 7\n"
    << "  --mod-cycles <value>      Modulator cycles across the plot. Default: 1\n"
    << "  --mod-depth <0..1>        Volume modulation depth. Default: 0.72\n"
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
      options.dimensions = adt::dimensionsForPreset(value);
    }
    else if (arg == "--width" && nextValue(args, &i, &value)) {
      options.dimensions.width = parseInt(value, arg);
    }
    else if (arg == "--height" && nextValue(args, &i, &value)) {
      options.dimensions.height = parseInt(value, arg);
    }
    else if (arg == "--fps" && nextValue(args, &i, &value)) {
      options.fps = parseDouble(value, arg);
    }
    else if (arg == "--frames" && nextValue(args, &i, &value)) {
      options.frames = parseInt(value, arg);
      options.sequence = options.frames > 1;
    }
    else if (arg == "--out" && nextValue(args, &i, &value)) {
      options.output_path = value;
      options.sequence = false;
    }
    else if (arg == "--out-dir" && nextValue(args, &i, &value)) {
      options.output_dir = value;
      options.sequence = true;
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

  if (options.dimensions.width <= 0 || options.dimensions.height <= 0)
    throw std::runtime_error("Width and height must be positive.");
  if (options.frames <= 0)
    throw std::runtime_error("Frame count must be positive.");
  if (options.fps <= 0.0)
    throw std::runtime_error("FPS must be positive.");

  return options;
}

std::filesystem::path framePath(const std::filesystem::path& directory, int frame) {
  std::ostringstream name;
  name << "frame_" << std::setw(4) << std::setfill('0') << frame << ".png";
  return directory / name.str();
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

    if (options.sequence) {
      std::filesystem::create_directories(options.output_dir);
      for (int frame = 0; frame < options.frames; ++frame) {
        const adt::Timeline timeline = adt::Timeline::forFrame(frame, options.frames, options.fps);
        const auto path = framePath(options.output_dir, frame);
        adt::saveVolumeModulatedSineFrame(path.string(), options.dimensions, timeline, {}, options.waveform);
      }
      std::cout << "Rendered " << options.frames << " PNG frames to "
                << options.output_dir.string() << "\n";
    }
    else {
      ensureParentDirectory(options.output_path);
      const adt::Timeline timeline = adt::Timeline::forFrame(0, 1, options.fps);
      adt::saveVolumeModulatedSineFrame(options.output_path.string(), options.dimensions, timeline,
                                        {}, options.waveform);
      std::cout << "Rendered " << options.output_path.string() << "\n";
    }
  }
  catch (const std::exception& e) {
    std::cerr << "adt_render: " << e.what() << "\n\n";
    printHelp();
    return 1;
  }

  return 0;
}
