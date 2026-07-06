#include "audio_diagram_tools/canonical_renderers.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace adt::canonical::renderers {
namespace {

using drawing::DrawContext;
using drawing::Rect;

constexpr float kPi = drawing::kPi;

constexpr uint32_t kBackground = 0xff1d1d1d;
constexpr uint32_t kPanelTop = 0xff3f3f3f;
constexpr uint32_t kPanelBottom = 0xff2e2e2e;
constexpr uint32_t kPanelStroke = 0xff595959;
constexpr uint32_t kPanelDarkStroke = 0xff171717;
constexpr uint32_t kBlockFillTop = 0xff333333;
constexpr uint32_t kBlockFillBottom = 0xff242424;
constexpr uint32_t kBlockActiveTop = 0xff3a453f;
constexpr uint32_t kBlockActiveBottom = 0xff29312d;
constexpr uint32_t kBlockOuterStroke = 0xff0c0c0c;
constexpr uint32_t kBlockBevel = 0xff5c5c5c;
constexpr uint32_t kBlockActiveBevel = 0xff78c991;
constexpr uint32_t kWave = 0xff8ed69f;
constexpr uint32_t kWaveShadow = 0xff0f130f;
constexpr uint32_t kMidline = 0xff474747;
constexpr uint32_t kLabel = 0xffb9beb9;
constexpr uint32_t kMutedLabel = 0xff9ea49f;
constexpr uint32_t kGlow = 0xff7acd93;

uint8_t channel(uint32_t color, int shift) {
  return static_cast<uint8_t>((color >> shift) & 0xffu);
}

uint32_t scaleAlpha(uint32_t color, float alpha_scale) {
  const uint8_t alpha = static_cast<uint8_t>(std::lround(
      static_cast<float>(channel(color, 24)) * drawing::clamp01(alpha_scale)));
  return (static_cast<uint32_t>(alpha) << 24) | (color & 0x00ffffffu);
}

uint32_t lerpColor(uint32_t a, uint32_t b, float t) {
  t = drawing::clamp01(t);
  const auto lerp_channel = [t](uint8_t from, uint8_t to) {
    return static_cast<uint8_t>(std::lround(static_cast<float>(from) +
                                           (static_cast<float>(to) - static_cast<float>(from)) * t));
  };

  return (static_cast<uint32_t>(lerp_channel(channel(a, 24), channel(b, 24))) << 24) |
         (static_cast<uint32_t>(lerp_channel(channel(a, 16), channel(b, 16))) << 16) |
         (static_cast<uint32_t>(lerp_channel(channel(a, 8), channel(b, 8))) << 8) |
         static_cast<uint32_t>(lerp_channel(channel(a, 0), channel(b, 0)));
}

float clamp01(float value) {
  return drawing::clamp01(value);
}

float ease(float value) {
  return drawing::smoothstep(0.0f, 1.0f, clamp01(value));
}

float interval(float value, float start, float end) {
  return clamp01((value - start) / (end - start));
}

float lerp(float a, float b, float t) {
  return a + (b - a) * t;
}

struct WavePoint {
  float x = 0.0f;
  float y = 0.0f;
  float value = 0.0f;
};

float processedValue(float local, float saturation, float gain) {
  const float sine = std::sin(2.0f * kPi * (local * 1.28f + 0.12f));
  const float bent = std::tanh(sine * 2.35f) / std::tanh(2.35f);
  return lerp(sine, bent, saturation) * gain * 0.84f;
}

std::vector<WavePoint> makeWavePoints(const Rect& plot,
                                      float draw_progress,
                                      float saturation,
                                      float gain) {
  draw_progress = clamp01(draw_progress);
  std::vector<WavePoint> points;
  if (draw_progress <= 0.002f)
    return points;

  constexpr int kSamples = 80;
  const int visible_samples =
      std::max(2, static_cast<int>(std::ceil(static_cast<float>(kSamples) * draw_progress)));
  points.reserve(visible_samples);
  const float center_y = plot.y + plot.height * 0.5f;
  const float amplitude = plot.height * 0.46f;

  for (int i = 0; i < visible_samples; ++i) {
    const float t = static_cast<float>(i) / static_cast<float>(kSamples - 1);
    const float local = std::min(t, draw_progress);
    const float value = processedValue(local, saturation, gain);
    points.push_back({
        plot.x + plot.width * local,
        center_y - value * amplitude,
        value,
    });
  }

  return points;
}

visage::Path wavePath(const std::vector<WavePoint>& points) {
  visage::Path path;
  for (size_t i = 0; i < points.size(); ++i) {
    if (i == 0)
      path.moveTo(points[i].x, points[i].y);
    else
      path.lineTo(points[i].x, points[i].y);
  }
  return path;
}

std::vector<visage::Path> waveFillPaths(const std::vector<WavePoint>& points, float center_y) {
  std::vector<visage::Path> fills;
  if (points.size() < 2)
    return fills;

  std::vector<WavePoint> lobe;
  lobe.push_back(points.front());

  const auto same_side = [](float a, float b) {
    return (a >= 0.0f && b >= 0.0f) || (a <= 0.0f && b <= 0.0f);
  };

  const auto add_lobe = [&](const std::vector<WavePoint>& source) {
    if (source.size() < 2)
      return;

    visage::Path fill;
    fill.moveTo(source.front().x, center_y);
    for (const WavePoint& point : source)
      fill.lineTo(point.x, point.y);
    fill.lineTo(source.back().x, center_y);
    fill.close();
    fills.push_back(fill);
  };

  for (size_t i = 1; i < points.size(); ++i) {
    const WavePoint& previous = points[i - 1];
    const WavePoint& point = points[i];
    if (!same_side(previous.value, point.value)) {
      const float denom = previous.value - point.value;
      const float t = std::abs(denom) < 0.00001f ? 0.0f : previous.value / denom;
      const WavePoint crossing { lerp(previous.x, point.x, t), center_y, 0.0f };
      lobe.push_back(crossing);
      add_lobe(lobe);
      lobe.clear();
      lobe.push_back(crossing);
    }

    lobe.push_back(point);
  }

  add_lobe(lobe);
  return fills;
}

void drawBlock(visage::Canvas& canvas,
               const Rect& block,
               float draw_progress,
               float saturation,
               float gain,
               float active_amount,
               float opacity) {
  opacity = clamp01(opacity);
  if (opacity <= 0.0f)
    return;

  active_amount = ease(active_amount);
  const bool has_wave = draw_progress > 0.01f;
  const float wave_amount = has_wave ? 1.0f : 0.0f;
  constexpr float kRadius = 2.75f;

  if (active_amount > 0.04f) {
    canvas.setColor(scaleAlpha(kGlow, active_amount * opacity * 0.14f));
    canvas.roundedRectangle(block.x - 3.0f,
                            block.y - 3.0f,
                            block.width + 6.0f,
                            block.height + 6.0f,
                            kRadius + 2.0f);
  }

  canvas.setColor(scaleAlpha(0xff000000, opacity * 0.22f));
  canvas.roundedRectangle(block.x + 2.0f, block.y + 2.0f, block.width, block.height, kRadius);

  const uint32_t fill_top =
      scaleAlpha(lerpColor(kBlockFillTop, kBlockActiveTop, active_amount * wave_amount), opacity);
  const uint32_t fill_bottom =
      scaleAlpha(lerpColor(kBlockFillBottom, kBlockActiveBottom, active_amount * wave_amount),
                 opacity);
  const uint32_t bevel =
      scaleAlpha(lerpColor(kBlockBevel, kBlockActiveBevel, active_amount * wave_amount),
                 opacity);

  canvas.setColor(scaleAlpha(kBlockOuterStroke, opacity));
  canvas.roundedRectangle(block.x, block.y, block.width, block.height, kRadius);
  canvas.setColor(visage::Brush::vertical(fill_top, fill_bottom));
  canvas.roundedRectangle(block.x + 1.0f,
                          block.y + 1.0f,
                          block.width - 2.0f,
                          block.height - 2.0f,
                          kRadius - 0.75f);
  canvas.setColor(bevel);
  canvas.roundedRectangleBorder(block.x + 0.5f,
                                block.y + 0.5f,
                                block.width - 1.0f,
                                block.height - 1.0f,
                                kRadius,
                                1.0f);
  canvas.setColor(scaleAlpha(0xff0b0b0b, opacity * 0.68f));
  canvas.roundedRectangleBorder(block.x + 3.0f,
                                block.y + 3.0f,
                                block.width - 6.0f,
                                block.height - 6.0f,
                                kRadius - 1.2f,
                                0.75f);

  const Rect plot { block.x + 8.0f, block.y + 8.0f, block.width - 16.0f, block.height - 16.0f };
  const float center_y = block.y + block.height * 0.5f;
  canvas.setColor(scaleAlpha(0xff161616, opacity * 0.38f));
  canvas.roundedRectangle(plot.x, plot.y, plot.width, plot.height, 1.4f);

  for (int i = 1; i < 4; ++i) {
    const float tick_x = plot.x + plot.width * static_cast<float>(i) / 4.0f;
    canvas.setColor(scaleAlpha(0xff3a3a3a, opacity * 0.72f));
    canvas.fill(tick_x - 0.45f, plot.y + 3.0f, 0.9f, plot.height - 6.0f);
  }

  drawing::drawLine(canvas,
                    plot.x + 1.0f,
                    center_y,
                    plot.x + plot.width - 1.0f,
                    center_y,
                    0.65f,
                    scaleAlpha(kMidline, opacity * 0.82f));

  const std::vector<WavePoint> points =
      makeWavePoints(plot, draw_progress, saturation, gain);
  if (points.size() >= 2) {
    canvas.setColor(scaleAlpha(kWave, opacity * 0.12f));
    for (const visage::Path& fill_path : waveFillPaths(points, center_y))
      canvas.fill(fill_path);

    const visage::Path wave = wavePath(points);
    drawing::fillStroke(canvas, wave, 2.8f, scaleAlpha(kWaveShadow, opacity * 0.78f));
    drawing::fillStroke(canvas, wave, 1.3f, scaleAlpha(kWave, opacity));
  }

  if (active_amount > 0.25f && draw_progress > 0.02f && draw_progress < 0.99f) {
    const float draw_x = plot.x + plot.width * draw_progress;
    drawing::drawLine(canvas,
                      draw_x,
                      plot.y + 2.0f,
                      draw_x,
                      plot.y + plot.height - 2.0f,
                      1.35f,
                      scaleAlpha(0xffbaf5c7, opacity * active_amount));
  }
}

void drawBackground(visage::Canvas& canvas, const Dimensions& dimensions) {
  canvas.setColor(kBackground);
  canvas.fill(0, 0, dimensions.width, dimensions.height);
}

void drawAudioEngine(visage::Canvas& canvas, float x, float y, float width, float height) {
  canvas.setColor(scaleAlpha(0xff000000, 0.20f));
  canvas.roundedRectangle(x + 3.0f, y + 3.0f, width, height, 5.0f);
  canvas.setColor(kPanelStroke);
  canvas.roundedRectangle(x, y, width, height, 5.0f);
  canvas.setColor(visage::Brush::vertical(kPanelTop, kPanelBottom));
  canvas.roundedRectangle(x + 2.0f, y + 2.0f, width - 4.0f, height - 4.0f, 3.5f);
  canvas.setColor(kPanelDarkStroke);
  canvas.roundedRectangleBorder(x + 2.5f, y + 2.5f, width - 5.0f, height - 5.0f, 3.5f, 1.0f);

  drawing::fauxBoldText(canvas,
                        "AUDIO ENGINE",
                        17.0f,
                        kLabel,
                        visage::Font::kCenter,
                        x,
                        y + 13.0f,
                        width,
                        24.0f);
}

void drawMiniNode(DrawContext& context,
                  const std::string& label,
                  const std::string& caption,
                  float x,
                  float y,
                  float scale,
                  float active_amount) {
  visage::Canvas& canvas = context.canvas;
  active_amount = ease(active_amount);
  const float width = 500.0f * scale;
  const float height = 357.0f * scale;

  if (active_amount > 0.03f) {
    canvas.setColor(scaleAlpha(kGlow, active_amount * 0.14f));
    canvas.roundedRectangle(x - 6.0f, y - 6.0f, width + 12.0f, height + 12.0f, 5.0f);
  }

  HiseNodeContainerOptions options;
  options.label = label;
  options.power_on = true;
  options.draw_close_button = false;
  drawHiseNodeContainerAt(context, options, x, y, scale);

  drawing::fauxBoldText(canvas,
                        caption,
                        14.5f,
                        active_amount > 0.1f ? lerpColor(kMutedLabel, kWave, active_amount) :
                                                kMutedLabel,
                        visage::Font::kCenter,
                        x - 6.0f,
                        y + height + 7.0f,
                        width + 12.0f,
                        22.0f);
}

struct BlockMotion {
  Rect rect;
  float draw_progress = 0.0f;
  float saturation = 0.0f;
  float gain = 1.0f;
  float active_amount = 0.0f;
  float opacity = 0.0f;
  float osc_active = 0.0f;
  float sat_active = 0.0f;
  float vol_active = 0.0f;
};

BlockMotion blockMotion(float phase,
                        const std::vector<std::pair<float, float>>& centers,
                        float block_width,
                        float block_height) {
  phase = phase - std::floor(phase);

  const auto point = [&](int index) {
    return centers[static_cast<size_t>(index)];
  };

  BlockMotion motion;
  motion.opacity = phase > 0.025f && phase < 0.975f ? 1.0f : 0.0f;

  float x = point(0).first;
  float y = point(0).second;
  auto set_between = [&](float start, float end, int from, int to) {
    const float t = ease(interval(phase, start, end));
    x = lerp(point(from).first, point(to).first, t);
    y = lerp(point(from).second, point(to).second, t);
  };
  auto hold = [&](int at) {
    x = point(at).first;
    y = point(at).second;
  };

  if (phase < 0.035f) {
    motion.opacity = 0.0f;
    hold(0);
  }
  else if (phase < 0.090f) {
    set_between(0.035f, 0.090f, 0, 1);
  }
  else if (phase < 0.150f) {
    set_between(0.090f, 0.150f, 1, 2);
  }
  else if (phase < 0.305f) {
    hold(2);
    const float t = ease(interval(phase, 0.150f, 0.305f));
    motion.draw_progress = t;
    motion.active_amount = 1.0f;
    motion.osc_active = 1.0f;
  }
  else if (phase < 0.360f) {
    set_between(0.305f, 0.360f, 2, 3);
    motion.draw_progress = 1.0f;
    motion.osc_active = 1.0f - interval(phase, 0.305f, 0.360f);
  }
  else if (phase < 0.430f) {
    set_between(0.360f, 0.430f, 3, 4);
    motion.draw_progress = 1.0f;
  }
  else if (phase < 0.490f) {
    set_between(0.430f, 0.490f, 4, 5);
    motion.draw_progress = 1.0f;
    motion.sat_active = interval(phase, 0.430f, 0.490f);
  }
  else if (phase < 0.610f) {
    hold(5);
    const float t = ease(interval(phase, 0.490f, 0.610f));
    motion.draw_progress = 1.0f;
    motion.saturation = t;
    motion.active_amount = 1.0f;
    motion.sat_active = 1.0f;
  }
  else if (phase < 0.665f) {
    set_between(0.610f, 0.665f, 5, 6);
    motion.draw_progress = 1.0f;
    motion.saturation = 1.0f;
    motion.sat_active = 1.0f - interval(phase, 0.610f, 0.665f);
  }
  else if (phase < 0.735f) {
    set_between(0.665f, 0.735f, 6, 7);
    motion.draw_progress = 1.0f;
    motion.saturation = 1.0f;
  }
  else if (phase < 0.795f) {
    set_between(0.735f, 0.795f, 7, 8);
    motion.draw_progress = 1.0f;
    motion.saturation = 1.0f;
    motion.vol_active = interval(phase, 0.735f, 0.795f);
  }
  else if (phase < 0.905f) {
    hold(8);
    const float t = ease(interval(phase, 0.795f, 0.905f));
    motion.draw_progress = 1.0f;
    motion.saturation = 1.0f;
    motion.gain = lerp(1.0f, 0.58f, t);
    motion.active_amount = 1.0f;
    motion.vol_active = 1.0f;
  }
  else if (phase < 0.950f) {
    set_between(0.905f, 0.950f, 8, 9);
    motion.draw_progress = 1.0f;
    motion.saturation = 1.0f;
    motion.gain = 0.58f;
    motion.vol_active = 1.0f - interval(phase, 0.905f, 0.950f);
  }
  else if (phase < 0.975f) {
    set_between(0.950f, 0.975f, 9, 10);
    motion.draw_progress = 1.0f;
    motion.saturation = 1.0f;
    motion.gain = 0.58f;
  }
  else {
    motion.opacity = 0.0f;
    hold(10);
    motion.draw_progress = 1.0f;
    motion.saturation = 1.0f;
    motion.gain = 0.58f;
  }

  motion.rect = { x - block_width * 0.5f, y - block_height * 0.5f, block_width, block_height };
  return motion;
}

void drawPluginChain(DrawContext& context,
                     const Dimensions& dimensions,
                     const Timeline& timeline,
                     const PluginChainRoutingOptions& options) {
  visage::Canvas& canvas = context.canvas;
  const float w = static_cast<float>(dimensions.width);

  if (options.clear_background)
    drawBackground(canvas, dimensions);

  constexpr float block_width = 76.0f;
  constexpr float block_height = 40.0f;
  const Rect engine { 72.0f, 58.0f, 660.0f, 96.0f };
  const float engine_y = engine.y + 70.0f;
  const float node_y = 302.0f;
  const float node_scale = 0.28f;
  const float node_w = 500.0f * node_scale;
  const float osc_cx = engine.x + 108.0f;
  const float sat_cx = engine.x + engine.width * 0.5f;
  const float vol_cx = engine.x + engine.width - 102.0f;
  const float osc_x = osc_cx - node_w * 0.5f;
  const float sat_x = sat_cx - node_w * 0.5f;
  const float vol_x = vol_cx - node_w * 0.5f;

  const float process_y = node_y + 50.0f;

  const std::vector<std::pair<float, float>> route_points {
    { -block_width - 28.0f, engine_y },
    { osc_cx, engine_y },
    { osc_cx, process_y },
    { osc_cx, engine_y },
    { sat_cx, engine_y },
    { sat_cx, process_y },
    { sat_cx, engine_y },
    { vol_cx, engine_y },
    { vol_cx, process_y },
    { vol_cx, engine_y },
    { w + block_width + 86.0f, engine_y },
  };

  const float phase =
      static_cast<float>(timeline.normalized_time - std::floor(timeline.normalized_time));
  const BlockMotion motion =
      blockMotion(phase, route_points, block_width, block_height);

  drawAudioEngine(canvas, engine.x, engine.y, engine.width, engine.height);

  drawing::fauxBoldText(canvas,
                        "TO SPEAKER",
                        19.0f,
                        kLabel,
                        visage::Font::kCenter,
                        engine.x + engine.width + 18.0f,
                        engine.y + 16.0f,
                        w - engine.x - engine.width - 28.0f,
                        26.0f);

  drawMiniNode(context, "OSC", "Oscillator", osc_x, node_y, node_scale, motion.osc_active);
  drawMiniNode(context, "SAT", "Saturator", sat_x, node_y, node_scale, motion.sat_active);
  drawMiniNode(context, "VOL", "Volume", vol_x, node_y, node_scale, motion.vol_active);

  if (motion.opacity > 0.0f) {
    drawBlock(canvas,
              motion.rect,
              motion.draw_progress,
              motion.saturation,
              motion.gain,
              motion.active_amount,
              motion.opacity);
  }
}

} // namespace

void drawPluginChainRoutingGraphic(DrawContext& context,
                                   const Dimensions& dimensions,
                                   const Timeline& timeline,
                                   const PluginChainRoutingOptions& options) {
  drawPluginChain(context, dimensions, timeline, options);
}

} // namespace adt::canonical::renderers
