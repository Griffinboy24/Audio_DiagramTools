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

void drawSpeakerConeMotionExperimentAt(DrawContext& context,
                                       const Dimensions& dimensions,
                                       const Timeline& timeline,
                                       float origin_x,
                                       float origin_y,
                                       float scale,
                                       bool clear_background,
                                       bool draw_caption,
                                       std::optional<float> drive_override,
                                       std::optional<float> sound_drive_override) {
  auto sx = [&](float x) { return origin_x + x * scale; };
  auto sy = [&](float y) { return origin_y + y * scale; };
  auto ss = [&](float value) { return value * scale; };

  constexpr uint32_t kInk = 0xff202836;
  constexpr uint32_t kBlue = 0xff7da2ff;

  const float phase = static_cast<float>(timeline.normalized_time);
  const float drive = drive_override.value_or(std::sin(phase * 2.0f * kPi));
  const float natural_sound_drive =
      0.86f * drive + 0.14f * std::sin((phase - 0.018f) * 2.0f * kPi);
  const float sound_drive = sound_drive_override.value_or(natural_sound_drive);

  const float left = 180.0f;
  const float top = 28.0f;
  constexpr float kSpeakerXPivot = 392.0f;
  constexpr float kSpeakerXScale = 0.86f;
  auto px = [&](float x) { return sx(left + kSpeakerXPivot + (x - kSpeakerXPivot) * kSpeakerXScale); };
  auto py = [&](float y) { return sy(top + y); };
  auto ps = [&](float value) { return ss(value); };

  visage::Canvas& canvas = context.canvas;
  if (clear_background) {
    canvas.setColor(visage::Brush::vertical(0xffffffff, 0xfff8f9fd));
    canvas.fill(0, 0, dimensions.width, dimensions.height);
  }

  visage::Region& speaker = addRegion(context, true);
  drawInRegion(context, speaker, [&](visage::Canvas& c) {
    const float cone_shift = 17.0f * drive;
    auto metalFill = [&](float x1, float y1, float x2, float y2) {
      return visage::Brush::linear(0xffeef3f8, 0xffc2ccd8,
                                   { px(x1), py(y1) }, { px(x2), py(y2) });
    };

    struct NormPoint {
      float x;
      float y;
    };

    constexpr Rect kAssemblyBounds { 450.0f, 355.0f, 133.0f, 267.0f };
    constexpr Rect kSpeakerBounds { 145.0f, 30.0f, 240.0f, 370.0f };
    auto pieceRect = [&](float x, float y, float width, float height) {
      return Rect {
        kSpeakerBounds.x + (x - kAssemblyBounds.x) / kAssemblyBounds.width * kSpeakerBounds.width,
        kSpeakerBounds.y + (y - kAssemblyBounds.y) / kAssemblyBounds.height * kSpeakerBounds.height,
        width / kAssemblyBounds.width * kSpeakerBounds.width,
        height / kAssemblyBounds.height * kSpeakerBounds.height,
      };
    };
    auto shiftedRect = [](Rect rect, float x_offset, float y_offset = 0.0f) {
      rect.x += x_offset;
      rect.y += y_offset;
      return rect;
    };
    auto appendContour = [&](visage::Path& path,
                             const Rect& rect,
                             std::initializer_list<NormPoint> points) {
      bool first = true;
      for (const NormPoint& point : points) {
        const float x = px(rect.x + point.x * rect.width);
        const float y = py(rect.y + point.y * rect.height);
        if (first) {
          path.moveTo(x, y);
          first = false;
        }
        else {
          path.lineTo(x, y);
        }
      }
      path.close();
    };
    auto tracedPath = [&](const Rect& rect,
                          std::initializer_list<std::initializer_list<NormPoint>> contours) {
      visage::Path path;
      path.setFillRule(visage::Path::FillRule::EvenOdd);
      for (auto contour : contours)
        appendContour(path, rect, contour);
      return path;
    };
    auto fillTracedPath = [&](const visage::Path& path,
                              const visage::Brush& fill,
                              float stroke_width = 1.45f) {
      c.setColor(fill);
      c.fill(path);
      fillStroke(c, path, ps(stroke_width), kInk);
    };

    auto drawRearMagnetBlock = [&](visage::Canvas& target) {
      target.setColor(visage::Brush::vertical(0xffd7dde7, 0xffaeb7c6));
      target.roundedRectangle(px(54.0f), py(112.0f), ps(76.0f), ps(184.0f), ps(8.0f));
      target.setColor(0x26ffffff);
      target.fill(px(58.0f), py(118.0f), ps(67.0f), ps(58.0f));
      target.setColor(0xffb7bfcc);
      target.roundedRectangleBorder(px(54.0f), py(112.0f), ps(76.0f), ps(184.0f), ps(8.0f),
                                    ps(2.0f));
      target.setColor(kInk);
      target.roundedRectangleBorder(px(54.0f), py(112.0f), ps(76.0f), ps(184.0f), ps(8.0f),
                                    ps(1.45f));
    };

    c.setColor(visage::Brush::vertical(0xffeef2f7, 0xffc8d0dc));
    c.roundedRectangle(px(33.0f), py(126.0f), ps(25.0f), ps(154.0f), ps(8.0f));
    c.setColor(kInk);
    c.roundedRectangleBorder(px(33.0f), py(126.0f), ps(25.0f), ps(154.0f), ps(8.0f), ps(1.45f));

    auto drawRearPoleBlock = [&](visage::Canvas& target) {
      visage::Path pole_side;
      pole_side.moveTo(px(122.0f), py(152.0f));
      pole_side.lineTo(px(158.0f), py(154.0f));
      pole_side.lineTo(px(158.0f), py(274.0f));
      pole_side.lineTo(px(122.0f), py(276.0f));
      pole_side.close();
      target.setColor(visage::Brush::linear(0xffc9d1dc, 0xff7f8b9c,
                                            { px(130.0f), py(214.0f) },
                                            { px(153.0f), py(214.0f) }));
      target.fill(pole_side);
      target.setColor(0x24ffffff);
      target.fill(px(134.0f), py(155.0f), ps(8.0f), ps(118.0f));
      target.setColor(kInk);
      fillStroke(target, pole_side, ps(1.45f), kInk);
    };

    const float moving_cone = cone_shift * 0.90f;
    const Rect coil_rect = shiftedRect(pieceRect(478.0f, 459.0f, 21.0f, 55.0f), moving_cone);
    const Rect frame_rect = pieceRect(450.0f, 355.0f, 108.0f, 258.0f);
    const Rect rim_rect = pieceRect(538.0f, 355.0f, 37.0f, 267.0f);

    drawRearPoleBlock(c);
    drawRearMagnetBlock(c);

    c.setColor(visage::Brush::vertical(0xffefbd7b, 0xffd38b49));
    c.fill(px(coil_rect.x), py(coil_rect.y), ps(coil_rect.width), ps(coil_rect.height));
    for (int i = 0; i < 7; ++i) {
      const float x = coil_rect.x + 2.1f + static_cast<float>(i) * coil_rect.width / 7.2f;
      drawLine(c, px(x), py(coil_rect.y), px(x), py(coil_rect.y + coil_rect.height), ps(1.15f),
               0xff65411f);
    }

    const float cone_x = moving_cone;
    const float cone_neck_x = coil_rect.x + coil_rect.width - 1.0f;
    const float cone_front_x = 386.0f + cone_x;
    const float cone_apex_x = 356.0f + cone_x;
    const float cone_top_y = 58.0f;
    const float cone_bottom_y = 370.0f;
    const float cone_mid_y = 214.0f;

    visage::Path cone_body;
    cone_body.moveTo(px(cone_neck_x), py(175.0f));
    cone_body.lineTo(px(cone_apex_x), py(cone_top_y));
    cone_body.bezierTo(px(cone_front_x - 2.0f), py(94.0f),
                       px(cone_front_x + 9.0f), py(145.0f),
                       px(cone_front_x + 10.0f), py(cone_mid_y));
    cone_body.bezierTo(px(cone_front_x + 9.0f), py(283.0f),
                       px(cone_front_x - 1.0f), py(336.0f),
                       px(cone_apex_x), py(cone_bottom_y));
    cone_body.lineTo(px(cone_neck_x), py(253.0f));
    cone_body.close();
    c.setColor(visage::Brush::linear(0xffc7d1dd, 0xfff3f6fb,
                                     { px(cone_neck_x), py(cone_mid_y) },
                                     { px(cone_front_x + 10.0f), py(cone_mid_y) }));
    c.fill(cone_body);
    fillStroke(c, cone_body, ps(1.35f), kInk);

    auto frameX = [&](float x) { return px(frame_rect.x + x * frame_rect.width); };
    auto frameY = [&](float y) { return py(frame_rect.y + y * frame_rect.height); };
    visage::Path basket_frame;
    basket_frame.setFillRule(visage::Path::FillRule::EvenOdd);
    basket_frame.moveTo(frameX(0.860f), frameY(0.060f));
    basket_frame.lineTo(frameX(0.952f), frameY(0.012f));
    basket_frame.lineTo(frameX(0.994f), frameY(0.090f));
    basket_frame.bezierTo(frameX(0.925f), frameY(0.176f),
                          frameX(0.900f), frameY(0.342f),
                          frameX(0.892f), frameY(0.500f));
    basket_frame.bezierTo(frameX(0.900f), frameY(0.658f),
                          frameX(0.925f), frameY(0.824f),
                          frameX(0.958f), frameY(0.936f));
    basket_frame.lineTo(frameX(0.910f), frameY(0.918f));
    basket_frame.lineTo(frameX(0.060f), frameY(0.748f));
    basket_frame.lineTo(frameX(0.005f), frameY(0.593f));
    basket_frame.lineTo(frameX(-0.005f), frameY(0.453f));
    basket_frame.lineTo(frameX(0.023f), frameY(0.329f));
    basket_frame.lineTo(frameX(0.069f), frameY(0.252f));
    basket_frame.lineTo(frameX(0.860f), frameY(0.060f));
    basket_frame.close();

    appendContour(basket_frame, frame_rect,
                  {
                    { 0.742f, 0.190f }, { 0.454f, 0.335f },
                    { 0.125f, 0.399f }, { 0.116f, 0.605f },
                    { 0.435f, 0.657f }, { 0.742f, 0.810f },
                    { 0.718f, 0.732f }, { 0.690f, 0.630f },
                    { 0.672f, 0.500f }, { 0.690f, 0.370f },
                    { 0.718f, 0.268f }, { 0.756f, 0.202f },
                  });
    appendContour(basket_frame, frame_rect,
                  {
                    { 0.778f, 0.095f }, { 0.176f, 0.238f },
                    { 0.134f, 0.329f }, { 0.389f, 0.293f },
                    { 0.444f, 0.277f }, { 0.708f, 0.174f },
                    { 0.792f, 0.101f },
                  });
    appendContour(basket_frame, frame_rect,
                  {
                    { 0.158f, 0.684f }, { 0.190f, 0.744f },
                    { 0.742f, 0.866f }, { 0.680f, 0.806f },
                    { 0.410f, 0.704f },
                  });
    visage::Region& basket_region = addRegion(context, true);
    basket_region.setNeedsLayer(true);
    drawInRegion(context, basket_region, [&](visage::Canvas& basket_canvas) {
      basket_canvas.setColor(metalFill(frame_rect.x, frame_rect.y,
                                       frame_rect.x + frame_rect.width,
                                       frame_rect.y + frame_rect.height));
      basket_canvas.fill(basket_frame);

      visage::Path basket_visible_outline;
      basket_visible_outline.moveTo(frameX(0.952f), frameY(0.012f));
      basket_visible_outline.lineTo(frameX(0.860f), frameY(0.060f));
      basket_visible_outline.lineTo(frameX(0.069f), frameY(0.252f));
      basket_visible_outline.lineTo(frameX(0.023f), frameY(0.329f));
      basket_visible_outline.lineTo(frameX(-0.005f), frameY(0.453f));
      basket_visible_outline.lineTo(frameX(0.005f), frameY(0.593f));
      basket_visible_outline.lineTo(frameX(0.060f), frameY(0.748f));
      basket_visible_outline.lineTo(frameX(0.910f), frameY(0.918f));
      fillStroke(basket_canvas, basket_visible_outline, ps(1.55f), kInk);

      visage::Path bottom_rim_join;
      bottom_rim_join.moveTo(frameX(0.896f), frameY(0.914f));
      bottom_rim_join.lineTo(frameX(0.960f), frameY(0.936f));
      fillStroke(basket_canvas, bottom_rim_join, ps(1.55f), kInk);

      auto strokeContour = [&](std::initializer_list<NormPoint> points) {
        visage::Path contour;
        appendContour(contour, frame_rect, points);
        fillStroke(basket_canvas, contour, ps(1.55f), kInk);
      };
      strokeContour({
          { 0.742f, 0.190f }, { 0.454f, 0.335f },
          { 0.125f, 0.399f }, { 0.116f, 0.605f },
          { 0.435f, 0.657f }, { 0.742f, 0.810f },
          { 0.718f, 0.732f }, { 0.690f, 0.630f },
          { 0.672f, 0.500f }, { 0.690f, 0.370f },
          { 0.718f, 0.268f }, { 0.756f, 0.202f },
      });
      strokeContour({
          { 0.778f, 0.095f }, { 0.176f, 0.238f },
          { 0.134f, 0.329f }, { 0.389f, 0.293f },
          { 0.444f, 0.277f }, { 0.708f, 0.174f },
          { 0.792f, 0.101f },
      });
      strokeContour({
          { 0.158f, 0.684f }, { 0.190f, 0.744f },
          { 0.742f, 0.866f }, { 0.680f, 0.806f },
          { 0.410f, 0.704f },
      });
    });

    const float cap_center_x = cone_front_x - 173.0f;
    const float cap_center_y = cone_mid_y;
    constexpr float kCapRadiusX = 116.0f;
    constexpr float kCapRadiusY = 88.0f;
    constexpr float kCapClipX = 312.0f;
    visage::Region& cap_region = addRegion(context, true);
    cap_region.setNeedsLayer(true);
    drawInRegion(context, cap_region, [&](visage::Canvas& cap_canvas) {
      const visage::Path cap = ellipsePath(px(cap_center_x), py(cap_center_y),
                                           ps(kCapRadiusX), ps(kCapRadiusY));
      cap_canvas.setColor(visage::Brush::linear(0xffc9d3df, 0xff657283,
                                                { px(cap_center_x - kCapRadiusX),
                                                  py(cap_center_y) },
                                                { px(cap_center_x + kCapRadiusX),
                                                  py(cap_center_y) }));
      cap_canvas.fill(cap);
      fillStroke(cap_canvas, cap, ps(1.15f), kInk);

      visage::Path hidden_by_rim;
      hidden_by_rim.moveTo(0.0f, 0.0f);
      hidden_by_rim.lineTo(px(kCapClipX), 0.0f);
      hidden_by_rim.bezierTo(px(kCapClipX - 7.0f), py(cone_mid_y - 55.0f),
                             px(kCapClipX - 7.0f), py(cone_mid_y + 55.0f),
                             px(kCapClipX), static_cast<float>(dimensions.height));
      hidden_by_rim.lineTo(0.0f, static_cast<float>(dimensions.height));
      hidden_by_rim.close();
      cap_canvas.setBlendMode(visage::BlendMode::MaskRemove);
      cap_canvas.setColor(0xffffffff);
      cap_canvas.fill(hidden_by_rim);
      cap_canvas.setBlendMode(visage::BlendMode::Alpha);
    });

    const visage::Path rim = tracedPath(rim_rect,
      { {
          { 0.455f, -0.002f }, { 0.919f, -0.002f }, { 0.986f, 0.004f },
          { 0.986f, 0.105f }, { 0.919f, 0.114f }, { 0.838f, 0.103f },
          { 0.700f, 0.122f }, { 0.405f, 0.236f }, { 0.286f, 0.371f },
          { 0.198f, 0.569f }, { 0.286f, 0.700f }, { 0.428f, 0.790f },
          { 0.770f, 0.888f }, { 0.838f, 0.893f }, { 0.919f, 0.882f },
          { 0.959f, 0.891f }, { 0.959f, 0.989f }, { 0.892f, 0.998f },
          { 0.622f, 0.998f }, { 0.527f, 0.981f }, { 0.470f, 0.910f },
          { 0.266f, 0.865f }, { 0.082f, 0.779f }, { -0.040f, 0.581f },
          { -0.040f, 0.412f }, { 0.044f, 0.296f }, { 0.264f, 0.157f },
          { 0.455f, 0.082f }, { 0.455f, 0.007f },
        } });
    fillTracedPath(rim, visage::Brush::linear(0xff687586, 0xff202a36,
                                              { px(rim_rect.x), py(rim_rect.y) },
                                              { px(rim_rect.x + rim_rect.width),
                                                py(rim_rect.y + rim_rect.height) }),
                   1.25f);

    visage::Region& rim_region = addRegion(context, true);
    drawInRegion(context, rim_region, [&](visage::Canvas& rim_canvas) {
      rim_canvas.setColor(visage::Brush::linear(0xff687586, 0xff202a36,
                                                { px(rim_rect.x), py(rim_rect.y) },
                                                { px(rim_rect.x + rim_rect.width),
                                                  py(rim_rect.y + rim_rect.height) }));
      rim_canvas.fill(rim);
      fillStroke(rim_canvas, rim, ps(1.25f), kInk);
    });
  });

  visage::Region& front_graphics = addRegion(context, true);
  drawInRegion(context, front_graphics, [&](visage::Canvas& c) {
    const float push = smoothstep(0.0f, 0.92f, std::max(sound_drive, 0.0f));
    const float energy = smoothstep(0.04f, 0.86f, std::abs(sound_drive));
    const float pulse = clamp01(0.34f + 0.52f * push + 0.14f * energy);
    auto byteAlpha = [](float value) {
      return static_cast<uint8_t>(std::clamp(value, 0.0f, 255.0f));
    };

    auto drawSoundArc = [&](float x,
                            float top_y,
                            float bottom_y,
                            float control_x,
                            float alpha_base,
                            float width_base,
                            float delay) {
      const float local = clamp01(pulse + delay * (push - 0.5f));
      const float expand = ss(6.0f * local);
      const uint32_t color = alphaColor(byteAlpha(alpha_base + 72.0f * local), kBlue);
      visage::Path arc;
      arc.moveTo(sx(x) + expand * 0.25f, sy(top_y));
      arc.bezierTo(sx(control_x) + expand, sy(194.0f),
                   sx(control_x) + expand, sy(296.0f),
                   sx(x) + expand * 0.25f, sy(bottom_y));
      fillStroke(c, arc, ss(width_base + 0.45f * local), color, { ss(6.0f), ss(9.0f) });
    };

    drawSoundArc(628.0f, 111.0f, 383.0f, 685.0f, 108.0f, 1.78f, 0.20f);
    drawSoundArc(658.0f, 79.0f, 415.0f, 725.0f, 88.0f, 1.62f, -0.08f);

    if (draw_caption) {
      fauxBoldText(c, "Top:", ss(31.0f), 0xff4167d6, visage::Font::kTopRight, sx(227.0f),
                   sy(474.0f), ss(115.0f), ss(38.0f));
      text(c, "Sound over time (waveform).", ss(29.0f), 0xff171b24, visage::Font::kTopLeft,
           sx(352.0f), sy(474.0f), ss(440.0f), ss(38.0f));
      fauxBoldText(c, "Bottom:", ss(31.0f), 0xff4167d6, visage::Font::kTopRight, sx(171.0f),
                   sy(524.0f), ss(166.0f), ss(38.0f));
      text(c, "Speaker cone follows that motion.", ss(29.0f), 0xff171b24, visage::Font::kTopLeft,
           sx(347.0f), sy(524.0f), ss(500.0f), ss(38.0f));
    }
  });
}


} // namespace adt::canonical::renderers
