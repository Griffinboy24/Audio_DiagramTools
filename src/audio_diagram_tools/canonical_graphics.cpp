#include "audio_diagram_tools/audio_file_motion.h"
#include "audio_diagram_tools/canonical_renderers.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <string_view>

namespace adt::canonical::renderers {
namespace {

void drawCanonicalGraphic(drawing::DrawContext& context,
                          std::string_view graphic_id,
                          const Dimensions& dimensions,
                          const Timeline& timeline,
                          const RenderOptions& options) {
  visage::Canvas& canvas = context.canvas;

  if (graphic_id == "array-graphic") {
    drawArrayGraphic(context, dimensions);
  }
  else if (graphic_id == "sample-values-plot") {
    drawSampleValuesPlot(context, dimensions);
  }
  else if (graphic_id == "double-arrow-graphic") {
    if (options.double_arrow.single_color)
      drawDoubleArrowGraphicWithColor(canvas, dimensions, *options.double_arrow.single_color);
    else
      drawDoubleArrowGraphic(canvas, dimensions);
  }
  else if (graphic_id == "audio-file-player-graphic") {
    constexpr float kReferenceWidth = 1422.0f;
    constexpr float kReferenceHeight = 632.0f;
    const float scale = std::min(static_cast<float>(dimensions.width) / kReferenceWidth,
                                 static_cast<float>(dimensions.height) / kReferenceHeight);
    const float origin_x = (static_cast<float>(dimensions.width) - kReferenceWidth * scale) * 0.5f;
    const float origin_y =
        (static_cast<float>(dimensions.height) - kReferenceHeight * scale) * 0.5f;
    const AudioFileSweep sweep = audioFileTwoStageSweep(timeline);
    const float playhead =
        options.audio_file_player.playhead_progress.value_or(sweep.playhead_progress);
    const bool erase_sweep = options.audio_file_player.playhead_progress ?
        options.audio_file_player.erase_sweep :
        sweep.erase_pass;
    drawAudioFilePlayerGraphicAt(context,
                                 dimensions,
                                 timeline,
                                 origin_x,
                                 origin_y,
                                 scale,
                                 options.audio_file_player.clear_background,
                                 options.audio_file_player.draw_waveform,
                                 playhead,
                                 erase_sweep);
  }
  else if (graphic_id == "speaker-animation-graphic") {
    constexpr float kReferenceWidth = 1020.0f;
    constexpr float kReferenceHeight = 592.0f;
    const float scale = std::min(static_cast<float>(dimensions.width) / kReferenceWidth,
                                 static_cast<float>(dimensions.height) / kReferenceHeight);
    const float origin_x = (static_cast<float>(dimensions.width) - kReferenceWidth * scale) * 0.5f;
    const float origin_y =
        (static_cast<float>(dimensions.height) - kReferenceHeight * scale) * 0.5f;
    drawSpeakerConeMotionExperimentAt(context,
                                      dimensions,
                                      timeline,
                                      origin_x,
                                      origin_y,
                                      scale,
                                      options.speaker_motion.clear_background,
                                      options.speaker_motion.draw_caption,
                                      options.speaker_motion.cone_drive,
                                      options.speaker_motion.sound_drive);
  }
  else if (graphic_id == "audio-file-to-speaker-scene") {
    drawAudioFileToSpeakerScene(context,
                                dimensions,
                                timeline,
                                options.audio_file_to_speaker);
  }
  else if (graphic_id == "hise-node-container") {
    drawHiseNodeContainer(context, dimensions, options.hise_node_container);
  }
  else {
    throw std::runtime_error("Unknown canonical graphic: " + std::string(graphic_id));
  }
}

} // namespace

visage::Screenshot renderFrame(std::string_view graphic_id,
                               const Dimensions& dimensions,
                               const Timeline& timeline,
                               const RenderOptions& options) {
  visage::Canvas canvas;
  canvas.setWindowless(dimensions.width, dimensions.height);
  canvas.updateTime(timeline.time_seconds);
  drawing::DrawContext context(canvas, dimensions);
  drawCanonicalGraphic(context, graphic_id, dimensions, timeline, options);
  canvas.submit();
  return canvas.takeScreenshot();
}

} // namespace adt::canonical::renderers
