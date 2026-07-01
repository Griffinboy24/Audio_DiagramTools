#include "audio_diagram_tools/composite_graphics.h"
#include "audio_diagram_tools/gif_exporter.h"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

struct RenderRequest {
  std::string graphic_id = std::string(adt::composites::ids::kSampleGainComparisonScene);
  adt::Dimensions dimensions { 0, 0 };
  bool list_graphics = false;
  bool render_gif = false;
  int frame_index = 0;
  int frame_count = 1;
  double fps = 18.0;
  std::filesystem::path output_path;
};

void printUsage() {
  std::cout
      << "Usage:\n"
      << "  adt_render_composite_graphic --list\n"
      << "  adt_render_composite_graphic --graphic sample-gain-comparison-scene "
         "--out artifacts/composites/sample-gain-comparison-scene.png\n\n"
      << "Options:\n"
      << "  --list                      List composite graphic IDs.\n"
      << "  --graphic <id>              Composite graphic to render.\n"
      << "  --width <px> --height <px>  Override render dimensions.\n"
      << "  --frame <index>             PNG frame index for animated composites.\n"
      << "  --frames <count>            Timeline frame count.\n"
      << "  --fps <rate>                Timeline frame rate.\n"
      << "  --gif                       Render an animated GIF instead of a PNG frame.\n"
      << "  --out <path>                Output PNG or GIF path.\n";
}

std::filesystem::path defaultOutputPath(const RenderRequest& request) {
  std::filesystem::path path =
      std::filesystem::path("artifacts") / "composites" / request.graphic_id;
  path.replace_extension(request.render_gif ? ".gif" : ".png");
  return path;
}

RenderRequest parseArgs(int argc, char** argv) {
  RenderRequest request;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    auto requireValue = [&](const std::string& option) -> std::string {
      if (i + 1 >= argc)
        throw std::runtime_error("Missing value for " + option);
      return argv[++i];
    };

    if (arg == "--help" || arg == "-h") {
      printUsage();
      std::exit(0);
    }
    else if (arg == "--list") {
      request.list_graphics = true;
    }
    else if (arg == "--graphic") {
      request.graphic_id = requireValue(arg);
    }
    else if (arg == "--width") {
      request.dimensions.width = std::stoi(requireValue(arg));
    }
    else if (arg == "--height") {
      request.dimensions.height = std::stoi(requireValue(arg));
    }
    else if (arg == "--frame") {
      request.frame_index = std::stoi(requireValue(arg));
    }
    else if (arg == "--frames") {
      request.frame_count = std::stoi(requireValue(arg));
    }
    else if (arg == "--fps") {
      request.fps = std::stod(requireValue(arg));
    }
    else if (arg == "--gif") {
      request.render_gif = true;
    }
    else if (arg == "--out") {
      request.output_path = requireValue(arg);
    }
    else {
      throw std::runtime_error("Unknown argument: " + arg);
    }
  }

  if (request.frame_count < 1)
    throw std::runtime_error("--frames must be at least 1.");
  if (request.fps <= 0.0)
    throw std::runtime_error("--fps must be positive.");

  if (request.output_path.empty())
    request.output_path = defaultOutputPath(request);

  return request;
}

void listGraphics() {
  for (const auto& graphic : adt::composites::compositeGraphics()) {
    std::cout << std::left << std::setw(34) << graphic.id
              << graphic.preferred_dimensions.width << "x"
              << graphic.preferred_dimensions.height << "  "
              << graphic.description << "\n";
  }
}

} // namespace

int main(int argc, char** argv) {
  try {
    const RenderRequest request = parseArgs(argc, argv);

    if (request.list_graphics) {
      listGraphics();
      return 0;
    }

    if (!adt::composites::compositeGraphicById(request.graphic_id))
      throw std::runtime_error("Unknown composite graphic: " + request.graphic_id);

    if (!request.output_path.parent_path().empty())
      std::filesystem::create_directories(request.output_path.parent_path());

    const adt::composition::Scene scene =
        adt::composites::compositeSceneById(request.graphic_id, request.dimensions);

    if (request.render_gif) {
      adt::GifExportSpec spec;
      spec.dimensions = scene.profile.dimensions;
      spec.frame_count = request.frame_count;
      spec.fps = request.fps;

      adt::saveAnimatedGif(request.output_path.string(), spec, [&](const adt::Timeline& timeline) {
        return adt::composition::renderSceneFrame(scene, timeline);
      });
    }
    else {
      const adt::Timeline timeline =
          adt::Timeline::forFrame(request.frame_index, request.frame_count, request.fps);
      adt::composition::saveSceneFrame(request.output_path.string(), scene, timeline);
    }

    std::cout << "Rendered " << request.graphic_id << " -> "
              << request.output_path.string() << "\n";
    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << "adt_render_composite_graphic: " << e.what() << "\n\n";
    printUsage();
    return 1;
  }
}
