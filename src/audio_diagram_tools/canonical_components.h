#pragma once

#include "audio_diagram_tools/render_types.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include <visage/graphics.h>

namespace adt::canonical {

struct CanonicalGraphic {
  std::string_view id;
  std::string_view description;
  Dimensions preferred_dimensions;
};

namespace ids {

inline constexpr std::string_view kArrayGraphic = "array-graphic";
inline constexpr std::string_view kSampleValuesPlot = "sample-values-plot";
inline constexpr std::string_view kDoubleArrowGraphic = "double-arrow-graphic";
inline constexpr std::string_view kAudioFilePlayerGraphic = "audio-file-player-graphic";
inline constexpr std::string_view kSpeakerAnimationGraphic = "speaker-animation-graphic";
inline constexpr std::string_view kAudioFileToSpeakerScene = "audio-file-to-speaker-scene";
inline constexpr std::string_view kHiseNodeContainer = "hise-node-container";

} // namespace ids

struct AudioFilePlayerOptions {
  bool draw_waveform = true;
  bool clear_background = true;
  std::optional<float> playhead_progress;
  bool erase_sweep = false;
};

struct SpeakerMotionOptions {
  bool draw_caption = true;
  bool clear_background = true;
  std::optional<float> cone_drive;
  std::optional<float> sound_drive;
};

struct DoubleArrowOptions {
  std::optional<uint32_t> single_color;
};

struct AudioFileToSpeakerSceneOptions {
  uint32_t arrow_color = 0xff4e4e4e;
  bool draw_caption = true;
};

struct HiseNodeContainerOptions {
  std::string label = "DSP";
  bool power_on = true;
  bool draw_close_button = true;
};

struct RenderOptions {
  AudioFilePlayerOptions audio_file_player;
  SpeakerMotionOptions speaker_motion;
  DoubleArrowOptions double_arrow;
  AudioFileToSpeakerSceneOptions audio_file_to_speaker;
  HiseNodeContainerOptions hise_node_container;
};

struct Component {
  std::string canonical_id;
  Dimensions dimensions;
  RenderOptions options;
};

const std::array<CanonicalGraphic, 7>& canonicalGraphics();
std::optional<CanonicalGraphic> canonicalGraphicById(std::string_view id);
Dimensions preferredDimensions(std::string_view id);

Component arrayGraphic(Dimensions dimensions = {});
Component sampleValuesPlot(Dimensions dimensions = {});
Component doubleArrowGraphic(const DoubleArrowOptions& options = {},
                             Dimensions dimensions = {});
Component audioFilePlayerGraphic(const AudioFilePlayerOptions& options = {},
                                 Dimensions dimensions = {});
Component speakerAnimationGraphic(const SpeakerMotionOptions& options = {},
                                  Dimensions dimensions = {});
Component audioFileToSpeakerScene(const AudioFileToSpeakerSceneOptions& options = {},
                                  Dimensions dimensions = {});
Component hiseNodeContainer(const HiseNodeContainerOptions& options = {},
                            Dimensions dimensions = {});

visage::Screenshot renderCanonicalGraphicFrame(std::string_view graphic_id,
                                               const Dimensions& dimensions,
                                               const Timeline& timeline = {},
                                               const RenderOptions& options = RenderOptions {});

void saveCanonicalGraphicFrame(const std::string& output_path,
                               std::string_view graphic_id,
                               const Dimensions& dimensions,
                               const Timeline& timeline = {},
                               const RenderOptions& options = RenderOptions {});

} // namespace adt::canonical
