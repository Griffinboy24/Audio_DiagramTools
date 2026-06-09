#include "audio_diagram_tools/style_lab_scene.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct CliOptions {
  adt::Dimensions dimensions = adt::dimensionsForPreset("blog-banner");
  std::string study = "warm-scope";
  std::filesystem::path output_path = "artifacts/style_lab/warm-scope.png";
  std::filesystem::path output_dir = "artifacts/style_lab";
  bool all = false;
  bool help = false;
  bool list_studies = false;
};

void printHelp() {
  std::cout
    << "Audio Diagram Tools style lab\n\n"
    << "Usage:\n"
    << "  adt_style_lab --all --out-dir artifacts/style_lab\n"
    << "  adt_style_lab --study filled-ribbon --out artifacts/style_lab/filled-ribbon.png\n\n"
    << "Options:\n"
    << "  --study <id>             Render one style study. Default: warm-scope\n"
    << "  --all                    Render every style study.\n"
    << "  --list-studies           List available style studies.\n"
    << "  --preset <id>            Use a named canvas preset. Default: blog-banner\n"
    << "  --width <px>             Output width override.\n"
    << "  --height <px>            Output height override.\n"
    << "  --out <png>              Single-study output path.\n"
    << "  --out-dir <directory>    Output directory for --all.\n"
    << "  --help                   Show this help.\n";
}

void printStudies() {
  std::cout << "Style studies:\n";
  for (const auto& study : adt::styleStudies())
    std::cout << "  " << study.id << "  " << study.description << "\n";
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
    else if (arg == "--list-studies") {
      options.list_studies = true;
    }
    else if (arg == "--all") {
      options.all = true;
    }
    else if (arg == "--study" && nextValue(args, &i, &value)) {
      options.study = value;
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
    else if (arg == "--out-dir" && nextValue(args, &i, &value)) {
      options.output_dir = value;
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

std::filesystem::path studyPath(const std::filesystem::path& directory, std::string_view study) {
  return directory / (std::string(study) + ".png");
}

} // namespace

int main(int argc, char** argv) {
  try {
    const CliOptions options = parseArgs(argc, argv);
    if (options.help) {
      printHelp();
      return 0;
    }
    if (options.list_studies) {
      printStudies();
      return 0;
    }

    const adt::Timeline timeline = adt::Timeline::forFrame(0, 1, 30.0);
    if (options.all) {
      std::filesystem::create_directories(options.output_dir);
      for (const auto& study : adt::styleStudies()) {
        const auto path = studyPath(options.output_dir, study.id);
        adt::saveStyleStudyFrame(path.string(), study.id, options.dimensions, timeline);
        std::cout << "Rendered " << path.string() << "\n";
      }
    }
    else {
      ensureParentDirectory(options.output_path);
      adt::saveStyleStudyFrame(options.output_path.string(), options.study, options.dimensions, timeline);
      std::cout << "Rendered " << options.output_path.string() << "\n";
    }
  }
  catch (const std::exception& e) {
    std::cerr << "adt_style_lab: " << e.what() << "\n\n";
    printHelp();
    return 1;
  }

  return 0;
}
