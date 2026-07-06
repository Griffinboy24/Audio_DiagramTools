#include "audio_diagram_tools/composite_graphics.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

namespace adt::composites {
namespace {

const std::array<CompositeGraphic, 7> kCompositeGraphics = {
  CompositeGraphic { ids::kSampleGainComparisonScene,
                     "HISE-width before/after sample gain comparison scene",
                     { 920, 700 } },
  CompositeGraphic { ids::kSampleArrayToPlotArticleScene,
                     "HISE-width article composite showing sample values becoming a plot",
                     { 920, 470 } },
  CompositeGraphic { ids::kSampleTablePlaybackArticleScene,
                     "HISE-width article composite for table/waveform playback",
                     { 920, 430 } },
  CompositeGraphic { ids::kWaveformBufferSplitArticleScene,
                     "HISE-width article composite for chunked waveform blocks",
                     { 920, 340 } },
  CompositeGraphic { ids::kWaveformVolumeScaleArticleScene,
                     "HISE-width article composite for waveform volume scaling",
                     { 920, 340 } },
  CompositeGraphic { ids::kPluginChainRoutingArticleScene,
                     "HISE-width article composite for buffer routing through plugins",
                     { 920, 560 } },
  CompositeGraphic { ids::kOutputStreamToSpeakerArticleScene,
                     "HISE-width article composite for processed output reaching speaker",
                     { 920, 300 } },
};

Dimensions dimensionsOrPreferred(std::string_view id, Dimensions dimensions) {
  if (dimensions.width > 0 && dimensions.height > 0)
    return dimensions;

  return preferredDimensions(id);
}

composition::TextRun centeredHeading(std::string label,
                                      float x,
                                      float y,
                                      float width) {
  composition::TextRun run;
  run.label = std::move(label);
  run.x = x;
  run.y = y;
  run.width = width;
  run.height = 42.0f;
  run.size = 29.0f;
  run.color = 0xff171b24;
  run.justification = visage::Font::kCenter;
  run.faux_bold = true;
  return run;
}

} // namespace

const std::array<CompositeGraphic, 7>& compositeGraphics() {
  return kCompositeGraphics;
}

std::optional<CompositeGraphic> compositeGraphicById(std::string_view id) {
  const auto& graphics = compositeGraphics();
  const auto it = std::find_if(graphics.begin(), graphics.end(),
                               [id](const CompositeGraphic& graphic) {
                                 return graphic.id == id;
                               });

  if (it == graphics.end())
    return std::nullopt;

  return *it;
}

Dimensions preferredDimensions(std::string_view id) {
  const std::optional<CompositeGraphic> graphic = compositeGraphicById(id);
  if (!graphic)
    throw std::runtime_error("Unknown composite graphic: " + std::string(id));

  return graphic->preferred_dimensions;
}

composition::Scene sampleGainComparisonScene(Dimensions dimensions) {
  dimensions = dimensionsOrPreferred(ids::kSampleGainComparisonScene, dimensions);
  if (dimensions.width < 860 || dimensions.height < 660)
    throw std::runtime_error("Sample gain comparison scene is too small to remain readable.");

  composition::Scene scene;
  scene.profile = composition::hiseArticleImageProfile(dimensions.height);
  scene.profile.dimensions = dimensions;

  constexpr Dimensions kPanelDimensions { 600, 241 };
  constexpr float kHeadingY = 24.0f;
  constexpr float kPanelY = 70.0f;
  constexpr float kSectionGap = 58.0f;

  const float panel_x =
      (static_cast<float>(dimensions.width) - static_cast<float>(kPanelDimensions.width)) * 0.5f;
  const float after_heading_y = kPanelY + static_cast<float>(kPanelDimensions.height) +
                                kSectionGap;
  const float after_panel_y = after_heading_y + 48.0f;

  canonical::SampleTableWaveformOptions before_options;
  before_options.clear_background = false;
  before_options.max_abs_value = 0.80f;

  canonical::SampleTableWaveformOptions after_options = before_options;
  for (float& value : after_options.values)
    value *= 0.5f;

  scene.text_runs.push_back(
      centeredHeading("Before", panel_x, kHeadingY, static_cast<float>(kPanelDimensions.width)));
  scene.text_runs.push_back(centeredHeading(
      "After x0.5", panel_x, after_heading_y, static_cast<float>(kPanelDimensions.width)));

  composition::PlacedGraphic before =
      composition::place(canonical::sampleTableWaveformGraphic(before_options, kPanelDimensions),
                         kPanelY);
  before.x = panel_x;
  scene.graphics.push_back(before);

  composition::PlacedGraphic after =
      composition::place(canonical::sampleTableWaveformGraphic(after_options, kPanelDimensions),
                         after_panel_y);
  after.x = panel_x;
  scene.graphics.push_back(after);

  return scene;
}

composition::Scene centeredArticleGraphicScene(Dimensions dimensions,
                                               const canonical::Component& component) {
  composition::Scene scene;
  scene.profile = composition::hiseArticleImageProfile(dimensions.height);
  scene.profile.dimensions = dimensions;

  composition::PlacedGraphic graphic =
      composition::place(component,
                         (static_cast<float>(dimensions.height) -
                          static_cast<float>(component.dimensions.height)) *
                             0.5f);
  graphic.x = (static_cast<float>(dimensions.width) -
               static_cast<float>(component.dimensions.width)) *
              0.5f;
  scene.graphics.push_back(graphic);
  return scene;
}

composition::Scene sampleArrayToPlotArticleScene(Dimensions dimensions) {
  dimensions = dimensionsOrPreferred(ids::kSampleArrayToPlotArticleScene, dimensions);

  composition::Scene scene;
  scene.profile = composition::hiseArticleImageProfile(dimensions.height);
  scene.profile.dimensions = dimensions;

  auto addCentered = [&](const canonical::Component& component, float y) {
    composition::PlacedGraphic graphic = composition::place(component, y);
    graphic.x = (static_cast<float>(dimensions.width) -
                 static_cast<float>(component.dimensions.width)) *
                0.5f;
    scene.graphics.push_back(graphic);
  };

  canonical::DoubleArrowOptions arrow_options;
  arrow_options.single_color = 0xff4e4e4e;

  addCentered(canonical::arrayGraphic({ 600, 140 }), 34.0f);
  addCentered(canonical::doubleArrowGraphic(arrow_options, { 46, 36 }), 159.0f);
  addCentered(canonical::sampleValuesPlot({ 600, 223 }), 198.0f);

  return scene;
}

composition::Scene sampleTablePlaybackArticleScene(Dimensions dimensions) {
  dimensions = dimensionsOrPreferred(ids::kSampleTablePlaybackArticleScene, dimensions);
  return centeredArticleGraphicScene(dimensions,
                                    canonical::sampleTablePlaybackScene({ 600, 339 }));
}

composition::Scene waveformBufferSplitArticleScene(Dimensions dimensions) {
  dimensions = dimensionsOrPreferred(ids::kWaveformBufferSplitArticleScene, dimensions);
  return centeredArticleGraphicScene(dimensions,
                                    canonical::waveformBufferSplitGraphic({ 600, 222 }));
}

composition::Scene waveformVolumeScaleArticleScene(Dimensions dimensions) {
  dimensions = dimensionsOrPreferred(ids::kWaveformVolumeScaleArticleScene, dimensions);
  return centeredArticleGraphicScene(dimensions,
                                    canonical::waveformVolumeScaleGraphic({ 600, 222 }));
}

composition::Scene pluginChainRoutingArticleScene(Dimensions dimensions) {
  dimensions = dimensionsOrPreferred(ids::kPluginChainRoutingArticleScene, dimensions);
  composition::Scene scene;
  scene.profile = composition::hiseDarkArticleImageProfile(dimensions.height);
  scene.profile.dimensions = dimensions;

  composition::PlacedGraphic graphic =
      composition::place(canonical::pluginChainRoutingGraphic({}, { 920, 520 }),
                         (static_cast<float>(dimensions.height) - 520.0f) * 0.5f);
  graphic.x = 0.0f;
  scene.graphics.push_back(graphic);
  return scene;
}

composition::Scene outputStreamToSpeakerArticleScene(Dimensions dimensions) {
  dimensions = dimensionsOrPreferred(ids::kOutputStreamToSpeakerArticleScene, dimensions);

  composition::Scene scene;
  scene.profile = composition::hiseDarkArticleImageProfile(dimensions.height);
  scene.profile.dimensions = dimensions;

  composition::PlacedGraphic graphic =
      composition::place(canonical::outputStreamToSpeakerGraphic({}, { 920, 300 }),
                         (static_cast<float>(dimensions.height) - 300.0f) * 0.5f);
  graphic.x = 0.0f;
  scene.graphics.push_back(graphic);
  return scene;
}

composition::Scene compositeSceneById(std::string_view id, Dimensions dimensions) {
  if (id == ids::kSampleGainComparisonScene)
    return sampleGainComparisonScene(dimensions);
  if (id == ids::kSampleArrayToPlotArticleScene)
    return sampleArrayToPlotArticleScene(dimensions);
  if (id == ids::kSampleTablePlaybackArticleScene)
    return sampleTablePlaybackArticleScene(dimensions);
  if (id == ids::kWaveformBufferSplitArticleScene)
    return waveformBufferSplitArticleScene(dimensions);
  if (id == ids::kWaveformVolumeScaleArticleScene)
    return waveformVolumeScaleArticleScene(dimensions);
  if (id == ids::kPluginChainRoutingArticleScene)
    return pluginChainRoutingArticleScene(dimensions);
  if (id == ids::kOutputStreamToSpeakerArticleScene)
    return outputStreamToSpeakerArticleScene(dimensions);

  throw std::runtime_error("Unknown composite graphic: " + std::string(id));
}

} // namespace adt::composites
