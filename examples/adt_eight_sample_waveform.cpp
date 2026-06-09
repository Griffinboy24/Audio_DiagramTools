#include "audio_diagram_tools/sampled_waveform_scene.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct CliOptions {
  adt::Dimensions dimensions = adt::dimensionsForPreset("blog-banner");
  std::filesystem::path output_path = "artifacts/science/eight-sample-waveform.png";
  bool help = false;
  bool list_presets = false;
};

void printHelp() {
  std::cout
    << "Audio Diagram Tools eight-sample waveform renderer\n\n"
    << "Usage:\n"
    << "  adt_eight_sample_waveform --out artifacts/science/eight-sample-waveform.png\n\n"
    << "Options:\n"
    << "  --preset <id>          Use a named canvas preset. Default: blog-banner\n"
    << "  --list-presets         List available canvas presets.\n"
    << "  --width <px>           Output width override.\n"
    << "  --height <px>          Output height override.\n"
    << "  --out <png>            Output path.\n"
    << "  --help                 Show this help.\n";
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
    else if (arg == "--out" && nextValue(args, &i, &value)) {
      options.output_path = value;
    }
    else {
      throw std::runtime_error("Unknown or incomplete option: " + arg);
    }
  }

  if (options.dimensions.width <= 0 || options.dimensions.height <= 0)
    throw std::runtime_error("Width and height must be positive.");

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
    adt::saveEightSampleWaveformFrame(options.output_path.string(), options.dimensions);
    std::cout << "Rendered " << options.output_path.string() << "\n";
  }
  catch (const std::exception& e) {
    std::cerr << "adt_eight_sample_waveform: " << e.what() << "\n\n";
    printHelp();
    return 1;
  }

  return 0;
}
