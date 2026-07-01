#pragma once

#include "audio_diagram_tools/composition.h"
#include "audio_diagram_tools/render_types.h"

#include <array>
#include <optional>
#include <string_view>

namespace adt::composites {

struct CompositeGraphic {
  std::string_view id;
  std::string_view description;
  Dimensions preferred_dimensions;
};

namespace ids {

inline constexpr std::string_view kSampleGainComparisonScene =
    "sample-gain-comparison-scene";
inline constexpr std::string_view kSampleArrayToPlotArticleScene =
    "sample-array-to-plot-article-scene";
inline constexpr std::string_view kSampleTablePlaybackArticleScene =
    "sample-table-playback-article-scene";
inline constexpr std::string_view kWaveformBufferSplitArticleScene =
    "waveform-buffer-split-article-scene";
inline constexpr std::string_view kWaveformVolumeScaleArticleScene =
    "waveform-volume-scale-article-scene";

} // namespace ids

const std::array<CompositeGraphic, 5>& compositeGraphics();
std::optional<CompositeGraphic> compositeGraphicById(std::string_view id);
Dimensions preferredDimensions(std::string_view id);

composition::Scene sampleGainComparisonScene(Dimensions dimensions = { 0, 0 });
composition::Scene sampleArrayToPlotArticleScene(Dimensions dimensions = { 0, 0 });
composition::Scene sampleTablePlaybackArticleScene(Dimensions dimensions = { 0, 0 });
composition::Scene waveformBufferSplitArticleScene(Dimensions dimensions = { 0, 0 });
composition::Scene waveformVolumeScaleArticleScene(Dimensions dimensions = { 0, 0 });
composition::Scene compositeSceneById(std::string_view id, Dimensions dimensions = { 0, 0 });

} // namespace adt::composites
