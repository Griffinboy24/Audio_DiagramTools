#pragma once

#include "audio_diagram_tools/canonical_components.h"
#include "audio_diagram_tools/canonical_drawing.h"
#include "audio_diagram_tools/render_types.h"

#include <cstdint>
#include <optional>
#include <string_view>

#include <visage/graphics.h>

namespace adt::canonical::renderers {

void drawArrayGraphic(drawing::DrawContext& context, const Dimensions& dimensions);
void drawSampleValuesPlot(drawing::DrawContext& context, const Dimensions& dimensions);
void drawSampleTablePlaybackScene(drawing::DrawContext& context,
                                  const Dimensions& dimensions,
                                  const Timeline& timeline);
void drawSampleTableWaveformGraphic(drawing::DrawContext& context,
                                    const Dimensions& dimensions,
                                    const SampleTableWaveformOptions& options);
void drawDenseSampleWaveformGraphic(drawing::DrawContext& context,
                                    const Dimensions& dimensions);
void drawWaveformBufferSplitGraphic(drawing::DrawContext& context,
                                    const Dimensions& dimensions);
void drawWaveformVolumeScaleGraphic(drawing::DrawContext& context,
                                    const Dimensions& dimensions,
                                    const Timeline& timeline);
void drawDoubleArrowGraphic(visage::Canvas& canvas, const Dimensions& dimensions);
void drawDoubleArrowGraphicWithColor(visage::Canvas& canvas,
                                     const Dimensions& dimensions,
                                     uint32_t color);
void drawAudioFilePlayerGraphicAt(drawing::DrawContext& context,
                                  const Dimensions& dimensions,
                                  const Timeline& timeline,
                                  float origin_x,
                                  float origin_y,
                                  float scale,
                                  bool clear_background,
                                  bool draw_waveform,
                                  float playhead_progress,
                                  bool erase_sweep,
                                  AudioWaveformKind waveform_kind = AudioWaveformKind::LoopingFile,
                                  std::string_view label = {},
                                  bool draw_playhead = true);
void drawSpeakerConeMotionExperimentAt(drawing::DrawContext& context,
                                       const Dimensions& dimensions,
                                       const Timeline& timeline,
                                       float origin_x,
                                       float origin_y,
                                       float scale,
                                       bool clear_background,
                                       bool draw_caption,
                                       std::optional<float> cone_drive,
                                       std::optional<float> sound_drive,
                                       std::optional<uint32_t> sound_wave_color = std::nullopt,
                                       float sound_wave_alpha_scale = 1.0f,
                                       float sound_wave_follow_scale = 0.0f);
void drawAudioFileToSpeakerScene(drawing::DrawContext& context,
                                 const Dimensions& dimensions,
                                 const Timeline& timeline,
                                 const AudioFileToSpeakerSceneOptions& options);
void drawVoiceSampleToSpeakerScene(drawing::DrawContext& context,
                                   const Dimensions& dimensions,
                                   const Timeline& timeline,
                                   const AudioFileToSpeakerSceneOptions& options);
void drawHiseNodeContainer(drawing::DrawContext& context,
                           const Dimensions& dimensions,
                           const HiseNodeContainerOptions& options);
void drawHiseNodeContainerAt(drawing::DrawContext& context,
                             const HiseNodeContainerOptions& options,
                             float origin_x,
                             float origin_y,
                             float scale);
void drawBlockProcessingGraphic(drawing::DrawContext& context,
                                const Dimensions& dimensions,
                                const Timeline& timeline,
                                const BlockProcessingOptions& options);
void drawOscillatorBlockFactoryGraphic(drawing::DrawContext& context,
                                       const Dimensions& dimensions,
                                       const Timeline& timeline,
                                       const OscillatorBlockFactoryOptions& options);
void drawPluginChainRoutingGraphic(drawing::DrawContext& context,
                                   const Dimensions& dimensions,
                                   const Timeline& timeline,
                                   const PluginChainRoutingOptions& options);
void drawOutputStreamToSpeakerGraphic(drawing::DrawContext& context,
                                      const Dimensions& dimensions,
                                      const Timeline& timeline,
                                      const OutputStreamToSpeakerOptions& options);

visage::Screenshot renderFrame(std::string_view graphic_id,
                               const Dimensions& dimensions,
                               const Timeline& timeline,
                               const RenderOptions& options);

} // namespace adt::canonical::renderers
