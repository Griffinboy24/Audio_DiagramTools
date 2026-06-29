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

void drawAudioFilePlayerGraphicAt(DrawContext& context,
                                  const Dimensions& dimensions,
                                  const Timeline& timeline,
                                  float origin_x,
                                  float origin_y,
                                  float scale,
                                  bool clear_background,
                                  bool draw_waveform,
                                  float playhead_progress,
                                  bool erase_sweep) {
  constexpr Rect kReferenceOuter { 43.0f, 83.0f, 1322.0f, 371.0f };

  auto sx = [&](float x) { return origin_x + x * scale; };
  auto sy = [&](float y) { return origin_y + y * scale; };

  visage::Canvas& canvas = context.canvas;
  if (clear_background) {
    canvas.setColor(0xffffffff);
    canvas.fill(0, 0, dimensions.width, dimensions.height);
  }

  const Rect outer { sx(kReferenceOuter.x),
                     sy(kReferenceOuter.y),
                     kReferenceOuter.width * scale,
                     kReferenceOuter.height * scale };
  const float frame_rim = 3.8f * scale;
  const Rect bevel = insetRect(outer, frame_rim, frame_rim);
  const Rect content = insetRect(bevel, frame_rim, frame_rim);
  DiagramFrameLayout layout;
  layout.outer = outer;
  layout.bevel = bevel;
  layout.content = content;
  layout.radius = outer.height * 0.052f;

  const float plot_x_inset = 42.0f * scale;
  const float plot_top = content.y + 20.0f * scale;
  const float separator_y = content.y + content.height - 69.0f * scale;
  const Rect plot { content.x + plot_x_inset,
                    plot_top,
                    content.width - 2.0f * plot_x_inset,
                    separator_y - plot_top };
  layout.plot = plot;

  visage::Region& shadow = addBlurRegion(context, 9.5f * scale);
  drawInRegion(context, shadow, [&](visage::Canvas& shadow_canvas) {
    fillRoundedRectPath(shadow_canvas,
                        outer.x + 7.0f * scale,
                        outer.y + 11.5f * scale,
                        outer.width - 14.0f * scale,
                        outer.height,
                        layout.radius,
                        0x26040a10);
  });

  drawDiagramFrame(canvas, layout);

  (void)timeline;
  const float playhead_t = playhead_progress;
  const float playhead_x = plot.x + plot.width * playhead_t;
  const float trail_left = plot.x;
  const float trail_right = plot.x + plot.width;
  const float trail_top = plot.y;
  const float trail_bottom = separator_y;
  canvas.setColor(0x2f5f6fda);
  constexpr int kTrailSlices = 240;
  const float trail_start_x = erase_sweep ? playhead_x : trail_left;
  const float trail_width =
      std::max(0.0f, erase_sweep ? trail_right - playhead_x : playhead_x - trail_left);
  for (int i = 0; i < kTrailSlices; ++i) {
    const float start = trail_width * static_cast<float>(i) / static_cast<float>(kTrailSlices);
    const float end = trail_width * static_cast<float>(i + 1) / static_cast<float>(kTrailSlices);
    canvas.fill(trail_start_x + start, trail_top, end - start, trail_bottom - trail_top);
  }

  drawTimelineGrid(canvas, plot, 8);

  if (draw_waveform)
    drawPlayedFutureWaveformTrace(
        context, makeComplexAudioWaveform(plot, 1800), plot, scale, playhead_t, erase_sweep);
  drawTimelineZeroAxis(canvas, plot);

  text(canvas,
       "0:00",
       24.5f * scale,
       0xffeef2f8,
       visage::Font::kTopLeft,
       plot.x,
       separator_y + 16.0f * scale,
       110.0f * scale,
       40.0f * scale);
  text(canvas,
       "0:03",
       24.5f * scale,
       0xffeef2f8,
       visage::Font::kTopRight,
       plot.x + plot.width - 110.0f * scale,
       separator_y + 16.0f * scale,
       110.0f * scale,
       40.0f * scale);

  visage::Region& playhead_bloom = addBlurRegion(context, 2.4f * scale);
  drawInRegion(context, playhead_bloom, [&](visage::Canvas& region_canvas) {
    drawLine(region_canvas,
             playhead_x,
             content.y - 1.0f * scale,
             playhead_x,
             separator_y,
             3.35f * scale,
             0x5d7483dc);
    region_canvas.setColor(0x446f7edc);
    region_canvas.circle(playhead_x - 9.5f * scale,
                         content.y - 9.5f * scale,
                         19.0f * scale);
  });

  visage::Region& foreground = addRegion(context, true);
  drawInRegion(context, foreground, [&](visage::Canvas& region_canvas) {
    drawLine(region_canvas,
             playhead_x,
             content.y - 2.0f * scale,
             playhead_x,
             separator_y,
             2.35f * scale,
             0xffc2ccf4);

    const float handle_radius = 10.0f * scale;
    const float handle_center_y = content.y - 2.0f * scale;
    region_canvas.setColor(0xff071016);
    region_canvas.circle(playhead_x - handle_radius, handle_center_y - handle_radius,
                         handle_radius * 2.0f);
    region_canvas.setColor(0xff9fa8d5);
    region_canvas.ring(playhead_x - handle_radius, handle_center_y - handle_radius,
                       handle_radius * 2.0f, 3.0f * scale);
    region_canvas.setColor(0xff111824);
    region_canvas.circle(playhead_x - 4.8f * scale, handle_center_y - 4.8f * scale,
                         9.6f * scale);
  });

  drawDiagramFrameCorners(canvas, layout);
}


} // namespace adt::canonical::renderers
