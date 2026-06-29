#pragma once

#include "audio_diagram_tools/render_types.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <visage/graphics.h>
#include <visage_graphics/post_effects.h>

namespace adt::canonical::drawing {

inline constexpr float kPi = 3.14159265358979323846f;

struct Rect {
  float x = 0.0f;
  float y = 0.0f;
  float width = 0.0f;
  float height = 0.0f;
};

struct SignalPoint {
  float t = 0.0f;
  float x = 0.0f;
  float y = 0.0f;
  float value = 0.0f;
};

struct DrawContext {
  DrawContext(visage::Canvas& render_canvas, const Dimensions& render_dimensions);

  visage::Canvas& canvas;
  Dimensions dimensions;
  std::vector<std::unique_ptr<visage::Region>> regions;
  std::vector<std::unique_ptr<visage::BlurPostEffect>> blur_effects;
};

struct DiagramFrameLayout {
  Rect outer;
  Rect bevel;
  Rect content;
  Rect plot;
  float radius = 10.0f;
};

float clamp01(float value);
float smoothstep(float edge0, float edge1, float value);
float insetToward(float value, float target, float amount);
uint32_t alphaColor(uint8_t alpha, uint32_t rgb);
Rect insetRect(const Rect& rect, float x, float y);
visage::Font labelFont(float size);
void text(visage::Canvas& canvas,
          const std::string& label,
          float size,
          uint32_t color,
          visage::Font::Justification justification,
          float x,
          float y,
          float width,
          float height);
void fauxBoldText(visage::Canvas& canvas,
                  const std::string& label,
                  float size,
                  uint32_t color,
                  visage::Font::Justification justification,
                  float x,
                  float y,
                  float width,
                  float height);
void fillStroke(visage::Canvas& canvas,
                const visage::Path& path,
                float width,
                uint32_t color,
                const std::vector<float>& dash = {},
                float dash_offset = 0.0f,
                visage::Path::EndCap cap = visage::Path::EndCap::Round);
visage::Region& addRegion(DrawContext& context, bool on_top);
visage::Region& addBlurRegion(DrawContext& context, float blur_radius);
void drawInRegion(DrawContext& context,
                  visage::Region& region,
                  const std::function<void(visage::Canvas&)>& draw);
void drawLine(visage::Canvas& canvas,
              float x1,
              float y1,
              float x2,
              float y2,
              float width,
              uint32_t color,
              const std::vector<float>& dash = {});
void drawChevron(visage::Canvas& canvas,
                 float left_x,
                 float top_y,
                 float center_x,
                 float point_y,
                 float right_x,
                 float stroke_width,
                 uint32_t color);
visage::Path pathFromPoints(const std::vector<SignalPoint>& points,
                            float offset_x = 0.0f,
                            float offset_y = 0.0f);
std::vector<SignalPoint> waveformSegment(const std::vector<SignalPoint>& points,
                                         float start_t,
                                         float end_t);
std::vector<SignalPoint> makeComplexAudioWaveform(const Rect& area, int samples);
void drawGriffinWaveformTrace(DrawContext& context,
                              const std::vector<SignalPoint>& points,
                              const Rect& plot,
                              float scale);
void drawPlayedFutureWaveformTrace(DrawContext& context,
                                   const std::vector<SignalPoint>& points,
                                   const Rect& plot,
                                   float scale,
                                   float playhead_t,
                                   bool erase_pass = false);
void drawFrameCorner(visage::Canvas& canvas,
                     float x,
                     float y,
                     float horizontal,
                     float vertical,
                     int x_direction,
                     int y_direction);
void drawDiagramFrameCorners(visage::Canvas& canvas, const DiagramFrameLayout& layout);
void drawDiagramFrame(visage::Canvas& canvas, const DiagramFrameLayout& layout);
void fillRoundedRectPath(visage::Canvas& canvas,
                         float x,
                         float y,
                         float width,
                         float height,
                         float radius,
                         uint32_t color);
void fillRoundedRectPath(visage::Canvas& canvas,
                         float x,
                         float y,
                         float width,
                         float height,
                         float radius,
                         const visage::Brush& brush);
void drawPillDashedHorizontalRule(visage::Canvas& canvas,
                                  float x,
                                  float y,
                                  float width,
                                  float thickness,
                                  float dash_length,
                                  float gap,
                                  uint32_t color);
void drawDot(visage::Canvas& canvas, float x, float y, float radius, uint32_t color);
void drawOpenDot(visage::Canvas& canvas,
                 float x,
                 float y,
                 float radius,
                 uint32_t color,
                 float width);
void drawTimelineGrid(visage::Canvas& canvas, const Rect& plot, std::size_t sample_count);
void drawTimelineZeroAxis(visage::Canvas& canvas, const Rect& plot);
visage::Path ellipsePath(float center_x, float center_y, float radius_x, float radius_y);

} // namespace adt::canonical::drawing
