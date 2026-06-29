#include "audio_diagram_tools/canonical_components.h"
#include "audio_diagram_tools/gif_exporter.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

struct RenderRequest {
  std::string graphic_id = "audio-file-to-speaker-scene";
  adt::Dimensions dimensions { 920, 642 };
  bool use_preferred_size = false;
  bool list_graphics = false;
  bool render_gif = false;
  int frame_index = 0;
  int frame_count = 1;
  double fps = 18.0;
  std::filesystem::path output_path;
  adt::canonical::RenderOptions options;
};

void printUsage() {
  std::cout
      << "Usage:\n"
      << "  adt_render_graphic --list\n"
      << "  adt_render_graphic --graphic audio-file-to-speaker-scene --preferred-size "
         "--out artifacts/canonical/scene.png\n"
      << "  adt_render_graphic --graphic audio-file-to-speaker-scene --preferred-size "
         "--gif --frames 180 --fps 18 --out artifacts/canonical/scene.gif\n\n"
      << "Options:\n"
      << "  --list                      List canonical graphic IDs.\n"
      << "  --graphic <id>              Canonical graphic to render.\n"
      << "  --preferred-size            Use the graphic's canonical preferred dimensions.\n"
      << "  --width <px> --height <px>  Override render dimensions.\n"
      << "  --frame <index>             PNG frame index for animated graphics.\n"
      << "  --frames <count>            Timeline frame count.\n"
      << "  --fps <rate>                Timeline frame rate.\n"
      << "  --gif                       Render an animated GIF instead of a PNG frame.\n"
      << "  --out <path>                Output PNG or GIF path.\n"
      << "  --arrow-color <hex>         Override the audio-file scene / arrow color.\n"
      << "  --playhead <0..1>           Force audio file player playhead position.\n"
      << "  --erase-pass                Use the audio file player's erase sweep state.\n";
}

uint32_t parseRgb(std::string_view value) {
  std::string text(value);
  if (!text.empty() && text.front() == '#')
    text.erase(text.begin());

  if (text.size() != 6 && text.size() != 8)
    throw std::runtime_error("Expected a 6- or 8-digit hex color.");

  uint32_t parsed = static_cast<uint32_t>(std::stoul(text, nullptr, 16));
  if (text.size() == 6)
    parsed |= 0xff000000u;

  return parsed;
}

std::filesystem::path defaultOutputPath(const RenderRequest& request) {
  std::filesystem::path path = std::filesystem::path("artifacts") / "canonical" / request.graphic_id;
  path.replace_extension(request.render_gif ? ".gif" : ".png");
  return path;
}

void applyPreferredSize(RenderRequest& request) {
  if (!request.use_preferred_size)
    return;

  const auto graphic = adt::canonical::canonicalGraphicById(request.graphic_id);
  if (!graphic)
    throw std::runtime_error("Unknown canonical graphic: " + request.graphic_id);

  request.dimensions = graphic->preferred_dimensions;
}

RenderRequest parseArgs(int argc, char** argv) {
  RenderRequest request;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    auto requireValue = [&](std::string_view option) -> std::string {
      if (i + 1 >= argc)
        throw std::runtime_error("Missing value for " + std::string(option));
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
    else if (arg == "--preferred-size") {
      request.use_preferred_size = true;
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
    else if (arg == "--arrow-color") {
      const uint32_t color = parseRgb(requireValue(arg));
      request.options.double_arrow.single_color = color;
      request.options.audio_file_to_speaker.arrow_color = color;
    }
    else if (arg == "--playhead") {
      request.options.audio_file_player.playhead_progress = std::stof(requireValue(arg));
    }
    else if (arg == "--erase-pass") {
      request.options.audio_file_player.erase_sweep = true;
    }
    else {
      throw std::runtime_error("Unknown argument: " + arg);
    }
  }

  if (request.frame_count < 1)
    throw std::runtime_error("--frames must be at least 1.");

  if (request.fps <= 0.0)
    throw std::runtime_error("--fps must be positive.");

  applyPreferredSize(request);

  if (request.output_path.empty())
    request.output_path = defaultOutputPath(request);

  return request;
}

void listGraphics() {
  for (const auto& graphic : adt::canonical::canonicalGraphics()) {
    std::cout << std::left << std::setw(32) << graphic.id << graphic.preferred_dimensions.width
              << "x" << graphic.preferred_dimensions.height << "  " << graphic.description
              << "\n";
  }
}

} // namespace

int main(int argc, char** argv) {
  try {
    RenderRequest request = parseArgs(argc, argv);

    if (request.list_graphics) {
      listGraphics();
      return 0;
    }

    if (!adt::canonical::canonicalGraphicById(request.graphic_id))
      throw std::runtime_error("Unknown canonical graphic: " + request.graphic_id);

    if (!request.output_path.parent_path().empty())
      std::filesystem::create_directories(request.output_path.parent_path());

    if (request.render_gif) {
      adt::GifExportSpec spec;
      spec.dimensions = request.dimensions;
      spec.frame_count = request.frame_count;
      spec.fps = request.fps;

      adt::saveAnimatedGif(request.output_path.string(), spec, [&](const adt::Timeline& timeline) {
        return adt::canonical::renderCanonicalGraphicFrame(
            request.graphic_id, request.dimensions, timeline, request.options);
      });
    }
    else {
      const adt::Timeline timeline =
          adt::Timeline::forFrame(request.frame_index, request.frame_count, request.fps);
      adt::canonical::saveCanonicalGraphicFrame(
          request.output_path.string(), request.graphic_id, request.dimensions, timeline, request.options);
    }

    std::cout << "Rendered " << request.graphic_id << " -> " << request.output_path.string()
              << "\n";
    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << "adt_render_graphic: " << e.what() << "\n\n";
    printUsage();
    return 1;
  }
}
