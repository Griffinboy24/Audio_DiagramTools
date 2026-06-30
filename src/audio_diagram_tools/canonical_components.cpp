#include "audio_diagram_tools/canonical_components.h"

#include "audio_diagram_tools/canonical_renderers.h"
#include "audio_diagram_tools/png_export.h"

#include <algorithm>
#include <stdexcept>

namespace adt::canonical {
namespace {

const std::array<CanonicalGraphic, 10> kCanonicalGraphics = {
  CanonicalGraphic { ids::kArrayGraphic,
                     "Canonical simple unlabelled Griffinboy array/table graphic",
                     { 1668, 388 } },
  CanonicalGraphic { ids::kSampleValuesPlot,
                     "Canonical pale discrete sample-value plot card",
                     { 1644, 612 } },
  CanonicalGraphic { ids::kSampleTablePlaybackScene,
                     "Canonical HISE-width animated table-to-reconstructed-waveform scene",
                     { 920, 520 } },
  CanonicalGraphic { ids::kDoubleArrowGraphic,
                     "Canonical compact double-down transition arrow",
                     { 346, 270 } },
  CanonicalGraphic { ids::kAudioFilePlayerGraphic,
                     "Canonical Griffinboy audio file player with animatable playhead",
                     { 1422, 632 } },
  CanonicalGraphic { ids::kSpeakerAnimationGraphic,
                     "Canonical speaker cone animation graphic with annotations",
                     { 1020, 592 } },
  CanonicalGraphic { ids::kAudioFileToSpeakerScene,
                     "Canonical HISE-width compound scene mapping a file waveform to speaker motion",
                     { 920, 642 } },
  CanonicalGraphic { ids::kVoiceSampleToSpeakerScene,
                     "Canonical HISE-width compound scene mapping a voice sample to speaker motion",
                     { 920, 642 } },
  CanonicalGraphic { ids::kHiseNodeContainer,
                     "Canonical HISE-style dark node/container with editable label",
                     { 500, 357 } },
  CanonicalGraphic { ids::kBlockProcessingGraphic,
                     "Canonical block-processing DSP animation graphic",
                     { 920, 330 } },
};

Dimensions dimensionsOrPreferred(std::string_view id, Dimensions dimensions) {
  if (dimensions.width > 0 && dimensions.height > 0)
    return dimensions;

  return preferredDimensions(id);
}

Component makeComponent(std::string_view id, Dimensions dimensions, RenderOptions options) {
  return { std::string(id), dimensionsOrPreferred(id, dimensions), options };
}

} // namespace

const std::array<CanonicalGraphic, 10>& canonicalGraphics() {
  return kCanonicalGraphics;
}

std::optional<CanonicalGraphic> canonicalGraphicById(std::string_view id) {
  const auto& graphics = canonicalGraphics();
  const auto it = std::find_if(graphics.begin(), graphics.end(),
                               [id](const CanonicalGraphic& graphic) {
    return graphic.id == id;
  });

  if (it == graphics.end())
    return std::nullopt;

  return *it;
}

Dimensions preferredDimensions(std::string_view id) {
  const std::optional<CanonicalGraphic> graphic = canonicalGraphicById(id);
  if (!graphic)
    throw std::runtime_error("Unknown canonical graphic: " + std::string(id));

  return graphic->preferred_dimensions;
}

Component arrayGraphic(Dimensions dimensions) {
  return makeComponent(ids::kArrayGraphic, dimensions, {});
}

Component sampleValuesPlot(Dimensions dimensions) {
  return makeComponent(ids::kSampleValuesPlot, dimensions, {});
}

Component sampleTablePlaybackScene(Dimensions dimensions) {
  return makeComponent(ids::kSampleTablePlaybackScene, dimensions, {});
}

Component doubleArrowGraphic(const DoubleArrowOptions& options, Dimensions dimensions) {
  RenderOptions render_options;
  render_options.double_arrow = options;
  return makeComponent(ids::kDoubleArrowGraphic, dimensions, render_options);
}

Component audioFilePlayerGraphic(const AudioFilePlayerOptions& options, Dimensions dimensions) {
  RenderOptions render_options;
  render_options.audio_file_player = options;
  return makeComponent(ids::kAudioFilePlayerGraphic, dimensions, render_options);
}

Component speakerAnimationGraphic(const SpeakerMotionOptions& options, Dimensions dimensions) {
  RenderOptions render_options;
  render_options.speaker_motion = options;
  return makeComponent(ids::kSpeakerAnimationGraphic, dimensions, render_options);
}

Component audioFileToSpeakerScene(const AudioFileToSpeakerSceneOptions& options,
                                  Dimensions dimensions) {
  RenderOptions render_options;
  render_options.audio_file_to_speaker = options;
  return makeComponent(ids::kAudioFileToSpeakerScene, dimensions, render_options);
}

Component voiceSampleToSpeakerScene(const AudioFileToSpeakerSceneOptions& options,
                                    Dimensions dimensions) {
  RenderOptions render_options;
  render_options.audio_file_to_speaker = options;
  return makeComponent(ids::kVoiceSampleToSpeakerScene, dimensions, render_options);
}

Component hiseNodeContainer(const HiseNodeContainerOptions& options, Dimensions dimensions) {
  RenderOptions render_options;
  render_options.hise_node_container = options;
  return makeComponent(ids::kHiseNodeContainer, dimensions, render_options);
}

Component blockProcessingGraphic(const BlockProcessingOptions& options, Dimensions dimensions) {
  RenderOptions render_options;
  render_options.block_processing = options;
  return makeComponent(ids::kBlockProcessingGraphic, dimensions, render_options);
}

visage::Screenshot renderCanonicalGraphicFrame(std::string_view graphic_id,
                                               const Dimensions& dimensions,
                                               const Timeline& timeline,
                                               const RenderOptions& options) {
  if (dimensions.width <= 0 || dimensions.height <= 0)
    throw std::runtime_error("Canonical graphic dimensions must be positive.");

  return renderers::renderFrame(graphic_id, dimensions, timeline, options);
}

void saveCanonicalGraphicFrame(const std::string& output_path,
                               std::string_view graphic_id,
                               const Dimensions& dimensions,
                               const Timeline& timeline,
                               const RenderOptions& options) {
  savePngWithStraightAlpha(
      output_path, renderCanonicalGraphicFrame(graphic_id, dimensions, timeline, options));
}

} // namespace adt::canonical
