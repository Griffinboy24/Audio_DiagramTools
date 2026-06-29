#include "audio_diagram_tools/canonical_renderers.h"
#include "audio_diagram_tools/audio_file_motion.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace adt::canonical::renderers {
using namespace drawing;

void drawAudioFileToSpeakerScene(DrawContext& context,
                                 const Dimensions& dimensions,
                                 const Timeline& timeline,
                                 const canonical::AudioFileToSpeakerSceneOptions& options =
                                     canonical::AudioFileToSpeakerSceneOptions {}) {
  constexpr float kContentWidth = 800.0f;
  constexpr float kContentHeight = 626.0f;
  constexpr float kReferenceWidth = 920.0f;
  constexpr float kReferenceHeight = 642.0f;
  constexpr Rect kPlayerOuter { 117.5f, 49.0f, 565.0f, 159.0f };
  constexpr float kAudioReferenceOuterX = 43.0f;
  constexpr float kAudioReferenceOuterY = 83.0f;
  constexpr float kAudioReferenceOuterWidth = 1322.0f;
  constexpr float kSpeakerReferenceWidth = 1020.0f;
  constexpr float kContentInsetX = (kReferenceWidth - kContentWidth) * 0.5f;
  constexpr float kContentInsetY = (kReferenceHeight - kContentHeight) * 0.5f;

  const float scene_scale = std::min(static_cast<float>(dimensions.width) / kReferenceWidth,
                                     static_cast<float>(dimensions.height) / kReferenceHeight);
  const float scene_origin_x =
      (static_cast<float>(dimensions.width) - kReferenceWidth * scene_scale) * 0.5f;
  const float scene_origin_y =
      (static_cast<float>(dimensions.height) - kReferenceHeight * scene_scale) * 0.5f;
  auto sx = [&](float x) { return scene_origin_x + x * scene_scale; };
  auto cx = [&](float x) { return sx(kContentInsetX + x); };
  auto sy = [&](float y) { return scene_origin_y + (kContentInsetY + y) * scene_scale; };
  auto ss = [&](float value) { return value * scene_scale; };

  visage::Canvas& canvas = context.canvas;
  canvas.setColor(0xffffffff);
  canvas.fill(0, 0, dimensions.width, dimensions.height);

  const float player_scale = kPlayerOuter.width / kAudioReferenceOuterWidth * scene_scale;
  const float player_origin_x = cx(kPlayerOuter.x) - kAudioReferenceOuterX * player_scale;
  const float player_origin_y = sy(kPlayerOuter.y) - kAudioReferenceOuterY * player_scale;
  const AudioFileSweep sweep = audioFileTwoStageSweep(timeline);
  drawAudioFilePlayerGraphicAt(
      context,
      dimensions,
      timeline,
      player_origin_x,
      player_origin_y,
      player_scale,
      false,
      true,
      sweep.playhead_progress,
      sweep.erase_pass);

  const float arrow_center_x = cx(kContentWidth * 0.5f);
  drawChevron(canvas,
              arrow_center_x - ss(10.5f),
              sy(244.0f),
              arrow_center_x,
              sy(254.0f),
              arrow_center_x + ss(10.5f),
              ss(4.2f),
              options.arrow_color);
  drawChevron(canvas,
              arrow_center_x - ss(10.5f),
              sy(257.0f),
              arrow_center_x,
              sy(267.0f),
              arrow_center_x + ss(10.5f),
              ss(4.2f),
              options.arrow_color);

  constexpr float kSpeakerScale = 0.487f;
  const float speaker_scale = kSpeakerScale * scene_scale;
  const float speaker_origin_x =
      cx((kContentWidth - kSpeakerReferenceWidth * kSpeakerScale) * 0.5f + 42.0f);
  const float speaker_origin_y = sy(268.0f);
  const float waveform_drive =
      speakerDriveForAudioFileProgress(sweep.playhead_progress);
  const float sound_drive = waveform_drive;
  drawSpeakerConeMotionExperimentAt(
      context,
      dimensions,
      timeline,
      speaker_origin_x,
      speaker_origin_y,
      speaker_scale,
      false,
      false,
      waveform_drive,
      sound_drive);

  if (!options.draw_caption)
    return;

  visage::Region& caption = addRegion(context, true);
  drawInRegion(context, caption, [&](visage::Canvas& caption_canvas) {
    constexpr float kCaptionLabelSize = 23.0f;
    constexpr float kCaptionBodySize = 21.6f;
    constexpr float kCaptionCenterX = kContentWidth * 0.5f + 22.0f;
    constexpr float kCaptionHeight = 32.0f;
    constexpr float kTopLabelWidth = 56.0f;
    constexpr float kTopBodyWidth = 316.0f;
    constexpr float kBottomLabelWidth = 100.0f;
    constexpr float kBottomBodyWidth = 372.0f;
    constexpr float kTopRowWidth = kTopLabelWidth + kTopBodyWidth;
    constexpr float kBottomRowWidth = kBottomLabelWidth + kBottomBodyWidth;
    const float top_x = kCaptionCenterX - kTopRowWidth * 0.5f;
    const float bottom_x = kCaptionCenterX - kBottomRowWidth * 0.5f;

    fauxBoldText(caption_canvas, "Top:", ss(kCaptionLabelSize), 0xff4167d6,
                 visage::Font::kTopLeft, cx(top_x), sy(538.0f), ss(kTopLabelWidth),
                 ss(kCaptionHeight));
    text(caption_canvas,
         "Sound over time (waveform).",
         ss(kCaptionBodySize),
         0xff171b24,
         visage::Font::kTopLeft,
         cx(top_x + kTopLabelWidth),
         sy(538.0f),
         ss(kTopBodyWidth),
         ss(kCaptionHeight));
    fauxBoldText(caption_canvas, "Bottom:", ss(kCaptionLabelSize), 0xff4167d6,
                 visage::Font::kTopLeft, cx(bottom_x), sy(570.0f), ss(kBottomLabelWidth),
                 ss(kCaptionHeight));
    text(caption_canvas,
         "Speaker cone follows that motion.",
         ss(kCaptionBodySize),
         0xff171b24,
         visage::Font::kTopLeft,
         cx(bottom_x + kBottomLabelWidth),
         sy(570.0f),
         ss(kBottomBodyWidth),
         ss(kCaptionHeight));
  });
}

} // namespace adt::canonical::renderers
