#include "audio_diagram_tools/block_processing_experiment.h"
#include "audio_diagram_tools/gif_exporter.h"
#include "audio_diagram_tools/png_export.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct CliOptions {
  adt::Dimensions dimensions { 920, 520 };
  std::filesystem::path output_path = "artifacts/experiments/block-processing-prototype.png";
  std::filesystem::path gif_path;
  int frame = 96;
  int frames = 192;
  double fps = 24.0;
  bool render_gif = false;
  bool help = false;
};

void printHelp() {
  std::cout
      << "Block processing experiment renderer\n\n"
      << "Usage:\n"
      << "  adt_block_processing_experiment --out artifacts/experiments/block.png\n"
      << "  adt_block_processing_experiment --gif artifacts/experiments/block.gif --frames 126 --fps 18\n\n"
      << "Options:\n"
      << "  --width <px>      Output width. Default: 920\n"
      << "  --height <px>     Output height. Default: 520\n"
      << "  --frame <index>   Still frame index. Default: 96\n"
      << "  --frames <count>  GIF frame count. Default: 192\n"
      << "  --fps <value>     GIF frame rate. Default: 24\n"
      << "  --out <png>       Still PNG output path.\n"
      << "  --gif <gif>       Animated GIF output path.\n"
      << "  --help            Show this help.\n";
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
    else if (arg == "--width" && nextValue(args, &i, &value)) {
      options.dimensions.width = parseInt(value, arg);
    }
    else if (arg == "--height" && nextValue(args, &i, &value)) {
      options.dimensions.height = parseInt(value, arg);
    }
    else if (arg == "--frame" && nextValue(args, &i, &value)) {
      options.frame = parseInt(value, arg);
    }
    else if (arg == "--frames" && nextValue(args, &i, &value)) {
      options.frames = parseInt(value, arg);
    }
    else if (arg == "--fps" && nextValue(args, &i, &value)) {
      options.fps = parseDouble(value, arg);
    }
    else if (arg == "--out" && nextValue(args, &i, &value)) {
      options.output_path = value;
    }
    else if (arg == "--gif" && nextValue(args, &i, &value)) {
      options.gif_path = value;
      options.render_gif = true;
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

    ensureParentDirectory(options.output_path);
    const adt::Timeline still_timeline =
        adt::Timeline::forFrame(options.frame, options.frames, options.fps);
    adt::savePngWithStraightAlpha(
        options.output_path.string(),
        adt::experiments::renderBlockProcessingExperimentFrame(options.dimensions, still_timeline));
    std::cout << "Rendered still to " << options.output_path.string() << "\n";

    if (options.render_gif) {
      ensureParentDirectory(options.gif_path);
      adt::GifExportSpec spec;
      spec.dimensions = options.dimensions;
      spec.frame_count = options.frames;
      spec.fps = options.fps;
      adt::saveAnimatedGif(options.gif_path.string(), spec, [&](const adt::Timeline& timeline) {
        return adt::experiments::renderBlockProcessingExperimentFrame(options.dimensions, timeline);
      });
      std::cout << "Rendered GIF to " << options.gif_path.string() << "\n";
    }
  }
  catch (const std::exception& e) {
    std::cerr << "adt_block_processing_experiment: " << e.what() << "\n\n";
    printHelp();
    return 1;
  }

  return 0;
}
