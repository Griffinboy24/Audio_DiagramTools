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
namespace {

void drawAudioFileToSpeakerSceneImpl(DrawContext& context,
                                     const Dimensions& dimensions,
                                     const Timeline& timeline,
                                     const canonical::AudioFileToSpeakerSceneOptions& options,
                                     bool voice_sample) {
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
  const AudioFileSweep sweep =
      voice_sample ? voiceSampleOneShotSweep(timeline) : audioFileTwoStageSweep(timeline);
  drawAudioFilePlayerGraphicAt(
      context,
      dimensions,
      timeline,
      player_origin_x,
      player_origin_y,
      player_scale,
      false,
      sweep.show_waveform,
      sweep.playhead_progress,
      sweep.erase_pass,
      voice_sample ? AudioWaveformKind::VoiceSample : AudioWaveformKind::LoopingFile,
      std::string_view {},
      sweep.show_playhead);

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
  const float sample_drive = voice_sample ?
      speakerDriveForVoiceSampleProgress(sweep.playhead_progress) :
      speakerDriveForAudioFileProgress(sweep.playhead_progress);
  const bool voice_silent = voice_sample && !sweep.show_playhead;
  const float cone_drive = voice_silent ? 0.0f : sample_drive;
  const float sound_drive = voice_silent ? 0.0f : sample_drive;
  drawSpeakerConeMotionExperimentAt(
      context,
      dimensions,
      timeline,
      speaker_origin_x,
      speaker_origin_y,
      speaker_scale,
      false,
      false,
      cone_drive,
      sound_drive);

  if (!options.draw_caption)
    return;

  visage::Region& caption = addRegion(context, true);
  drawInRegion(context, caption, [&](visage::Canvas& caption_canvas) {
    constexpr float kCaptionLabelSize = 23.0f;
    constexpr float kCaptionBodySize = 21.6f;
    constexpr float kCaptionHeight = 32.0f;
    constexpr float kLabelBodyGap = 14.0f;
    const float caption_center_x = kContentWidth * 0.5f;
    const float label_body_gap = kLabelBodyGap;
    const float top_label_width = voice_sample ? 60.0f : 62.0f;
    const float top_body_width = voice_sample ? 244.0f : 316.0f;
    const float bottom_label_width = voice_sample ? 106.0f : 108.0f;
    const float bottom_body_width = voice_sample ? 92.0f : 372.0f;
    const float top_row_width = top_label_width + label_body_gap + top_body_width;
    const float bottom_row_width = bottom_label_width + label_body_gap + bottom_body_width;
    const float top_row_x = caption_center_x - top_row_width * 0.5f;
    const float bottom_row_x = caption_center_x - bottom_row_width * 0.5f;
    const std::string_view top_body =
        voice_sample ? "Vocal sample audio" : "Sound over time (waveform).";
    const std::string_view bottom_body =
        voice_sample ? "Speaker" : "Speaker cone follows that motion.";

    fauxBoldText(caption_canvas, "Top:", ss(kCaptionLabelSize), 0xff4167d6,
                 visage::Font::kTopRight, cx(top_row_x), sy(538.0f), ss(top_label_width),
                 ss(kCaptionHeight));
    text(caption_canvas,
         std::string(top_body),
         ss(kCaptionBodySize),
         0xff171b24,
         visage::Font::kTopLeft,
         cx(top_row_x + top_label_width + label_body_gap),
         sy(538.0f),
         ss(top_body_width),
         ss(kCaptionHeight));
    fauxBoldText(caption_canvas, "Bottom:", ss(kCaptionLabelSize), 0xff4167d6,
                 visage::Font::kTopRight, cx(bottom_row_x), sy(570.0f),
                 ss(bottom_label_width),
                 ss(kCaptionHeight));
    text(caption_canvas,
         std::string(bottom_body),
         ss(kCaptionBodySize),
         0xff171b24,
         visage::Font::kTopLeft,
         cx(bottom_row_x + bottom_label_width + label_body_gap),
         sy(570.0f),
         ss(bottom_body_width),
         ss(kCaptionHeight));
  });
}

} // namespace

void drawAudioFileToSpeakerScene(DrawContext& context,
                                 const Dimensions& dimensions,
                                 const Timeline& timeline,
                                 const canonical::AudioFileToSpeakerSceneOptions& options) {
  drawAudioFileToSpeakerSceneImpl(context, dimensions, timeline, options, false);
}

void drawVoiceSampleToSpeakerScene(DrawContext& context,
                                   const Dimensions& dimensions,
                                   const Timeline& timeline,
                                   const canonical::AudioFileToSpeakerSceneOptions& options) {
  drawAudioFileToSpeakerSceneImpl(context, dimensions, timeline, options, true);
}

} // namespace adt::canonical::renderers
