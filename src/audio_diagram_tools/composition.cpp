#include "audio_diagram_tools/composition.h"

#include "audio_diagram_tools/png_export.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

#include <embedded/fonts.h>

namespace adt::composition {
namespace {

uint8_t channel(uint32_t color, int shift) {
  return static_cast<uint8_t>((color >> shift) & 0xffu);
}

void fillScreenshot(visage::Screenshot& screenshot, uint32_t color) {
  uint8_t* data = screenshot.data();
  const int pixel_count = screenshot.width() * screenshot.height();
  const uint8_t red = channel(color, 16);
  const uint8_t green = channel(color, 8);
  const uint8_t blue = channel(color, 0);
  const uint8_t alpha = channel(color, 24);

  for (int pixel = 0; pixel < pixel_count; ++pixel) {
    uint8_t* rgba = data + pixel * 4;
    rgba[0] = red;
    rgba[1] = green;
    rgba[2] = blue;
    rgba[3] = alpha;
  }
}

void compositeSourceOver(visage::Screenshot& destination,
                         const visage::Screenshot& source,
                         int left,
                         int top) {
  const int start_x = std::max(0, left);
  const int start_y = std::max(0, top);
  const int end_x = std::min(destination.width(), left + source.width());
  const int end_y = std::min(destination.height(), top + source.height());

  if (start_x >= end_x || start_y >= end_y)
    return;

  uint8_t* dst_data = destination.data();
  const uint8_t* src_data = source.data();

  for (int y = start_y; y < end_y; ++y) {
    for (int x = start_x; x < end_x; ++x) {
      const int source_x = x - left;
      const int source_y = y - top;
      const uint8_t* src = src_data + (source_y * source.width() + source_x) * 4;
      uint8_t* dst = dst_data + (y * destination.width() + x) * 4;
      const uint8_t source_alpha = src[3];

      if (source_alpha == 255) {
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = 255;
        continue;
      }

      if (source_alpha == 0)
        continue;

      const uint32_t inverse_alpha = 255u - source_alpha;
      dst[0] = static_cast<uint8_t>((src[0] * source_alpha + dst[0] * inverse_alpha + 127u) / 255u);
      dst[1] = static_cast<uint8_t>((src[1] * source_alpha + dst[1] * inverse_alpha + 127u) / 255u);
      dst[2] = static_cast<uint8_t>((src[2] * source_alpha + dst[2] * inverse_alpha + 127u) / 255u);
      dst[3] = static_cast<uint8_t>(std::min<uint32_t>(
          255u, source_alpha + (static_cast<uint32_t>(dst[3]) * inverse_alpha + 127u) / 255u));
    }
  }
}

visage::Font labelFont(float size) {
  return visage::Font(size, visage::fonts::Lato_Regular_ttf);
}

void drawTextRun(visage::Canvas& canvas, const TextRun& run) {
  canvas.setColor(run.color);
  canvas.text(run.label.c_str(), labelFont(run.size), run.justification,
              run.x, run.y, run.width, run.height);
  if (run.faux_bold) {
    canvas.text(run.label.c_str(), labelFont(run.size), run.justification,
                run.x + 0.45f, run.y, run.width, run.height);
  }
}

visage::Screenshot renderTextLayer(const Profile& profile,
                                   const std::vector<TextRun>& text_runs,
                                   double time_seconds) {
  visage::Canvas canvas;
  canvas.setWindowless(profile.dimensions.width, profile.dimensions.height);
  canvas.updateTime(time_seconds);
  for (const TextRun& run : text_runs)
    drawTextRun(canvas, run);
  canvas.submit();
  return canvas.takeScreenshot();
}

} // namespace

Profile hiseInlineProfile(int height) {
  Profile profile;
  profile.id = "hise-inline";
  profile.description = "Current HISE inline article graphic target";
  profile.dimensions = hiseArticleImageDimensions(height);
  profile.content_width = 800.0f;
  profile.top_padding = 18.0f;
  profile.bottom_padding = 20.0f;
  profile.background = kHisePostBackground;
  return profile;
}

Profile hiseArticleImageProfile(int height) {
  return hiseInlineProfile(height);
}

Profile hiseDarkArticleImageProfile(int height) {
  Profile profile = hiseInlineProfile(height);
  profile.id = "hise-dark-inline";
  profile.description = "Current HISE inline article graphic target on Griffin dark background";
  profile.background = kGriffinDarkBackground;
  return profile;
}

float contentLeft(const Profile& profile) {
  return (static_cast<float>(profile.dimensions.width) - profile.content_width) * 0.5f;
}

float contentCenterX(const Profile& profile) {
  return static_cast<float>(profile.dimensions.width) * 0.5f;
}

Dimensions hiseArticleImageDimensions(int height) {
  return { 920, height };
}

float alignedX(const Profile& profile, const PlacedGraphic& graphic) {
  if (graphic.x)
    return *graphic.x;

  float x = contentLeft(profile);
  if (graphic.align == HorizontalAlign::Center)
    x += (profile.content_width - static_cast<float>(graphic.dimensions.width)) * 0.5f;
  else if (graphic.align == HorizontalAlign::Right)
    x += profile.content_width - static_cast<float>(graphic.dimensions.width);

  return x + graphic.x_offset;
}

PlacedGraphic place(const canonical::Component& component,
                    float y,
                    HorizontalAlign align,
                    float x_offset) {
  PlacedGraphic graphic;
  graphic.canonical_id = component.canonical_id;
  graphic.dimensions = component.dimensions;
  graphic.options = component.options;
  graphic.y = y;
  graphic.align = align;
  graphic.x_offset = x_offset;
  return graphic;
}

std::vector<TextRun> centeredLabelBodyLine(const Profile& profile,
                                           std::string label,
                                           std::string body,
                                           float y,
                                           float total_width,
                                           const LabelBodyLineOptions& options) {
  const float left = contentCenterX(profile) - total_width * 0.5f;
  const float body_width = total_width - options.label_width;

  TextRun label_run;
  label_run.label = std::move(label);
  label_run.x = left;
  label_run.y = y;
  label_run.width = options.label_width;
  label_run.height = options.height;
  label_run.size = options.label_size;
  label_run.color = options.label_color;
  label_run.justification = visage::Font::kTopRight;
  label_run.faux_bold = options.label_faux_bold;

  TextRun body_run;
  body_run.label = std::move(body);
  body_run.x = left + options.label_width;
  body_run.y = y;
  body_run.width = body_width;
  body_run.height = options.height;
  body_run.size = options.body_size;
  body_run.color = options.body_color;
  body_run.justification = visage::Font::kTopLeft;

  return { label_run, body_run };
}

void addCenteredLabelBodyLine(Scene& scene,
                              std::string label,
                              std::string body,
                              float y,
                              float total_width,
                              const LabelBodyLineOptions& options) {
  std::vector<TextRun> runs =
      centeredLabelBodyLine(scene.profile, std::move(label), std::move(body), y, total_width, options);
  scene.text_runs.insert(scene.text_runs.end(), runs.begin(), runs.end());
}

Scene verticalStack(Profile profile,
                    const std::vector<StackItem>& items,
                    const StackOptions& options) {
  float cursor_y = options.top_padding;
  Scene scene;
  scene.profile = profile;
  float last_gap = 0.0f;

  for (const StackItem& item : items) {
    scene.graphics.push_back(place(item.component, cursor_y, item.align, item.x_offset));
    cursor_y += static_cast<float>(item.component.dimensions.height);
    last_gap = item.gap_after > 0.0f ? item.gap_after : options.default_gap;
    cursor_y += last_gap;
  }

  if (!items.empty())
    cursor_y -= last_gap;

  cursor_y += options.bottom_padding;

  if (options.auto_height)
    scene.profile.dimensions.height = static_cast<int>(std::ceil(cursor_y));

  return scene;
}

StackBuilder::StackBuilder(Profile profile, StackOptions options) :
    options_(options), cursor_y_(options.top_padding) {
  scene_.profile = profile;
}

StackBuilder& StackBuilder::add(const canonical::Component& component,
                                float gap_after,
                                HorizontalAlign align,
                                float x_offset) {
  scene_.graphics.push_back(place(component, cursor_y_, align, x_offset));
  cursor_y_ += static_cast<float>(component.dimensions.height);
  last_gap_ = gap_after >= 0.0f ? gap_after : options_.default_gap;
  cursor_y_ += last_gap_;
  has_graphics_ = true;
  return *this;
}

StackBuilder& StackBuilder::addText(const TextRun& run) {
  scene_.text_runs.push_back(run);
  return *this;
}

float StackBuilder::cursorY() const {
  return cursor_y_;
}

Scene StackBuilder::build() const {
  Scene scene = scene_;
  float height = cursor_y_;

  if (has_graphics_)
    height -= last_gap_;

  height += options_.bottom_padding;

  if (options_.auto_height)
    scene.profile.dimensions.height = static_cast<int>(std::ceil(height));

  return scene;
}

visage::Screenshot renderSceneFrame(const Scene& scene, const Timeline& timeline) {
  if (scene.profile.dimensions.width <= 0 || scene.profile.dimensions.height <= 0)
    throw std::runtime_error("Composition scene dimensions must be positive.");

  visage::Canvas canvas;
  canvas.setWindowless(scene.profile.dimensions.width, scene.profile.dimensions.height);
  canvas.updateTime(timeline.time_seconds);
  canvas.setColor(scene.profile.background);
  canvas.fill(0, 0, scene.profile.dimensions.width, scene.profile.dimensions.height);
  canvas.submit();

  visage::Screenshot result = canvas.takeScreenshot();
  fillScreenshot(result, scene.profile.background);

  for (const PlacedGraphic& graphic : scene.graphics) {
    if (graphic.dimensions.width <= 0 || graphic.dimensions.height <= 0)
      throw std::runtime_error("Composition graphic dimensions must be positive.");

    visage::Screenshot layer = canonical::renderCanonicalGraphicFrame(
        graphic.canonical_id, graphic.dimensions, timeline, graphic.options);
    compositeSourceOver(result,
                        layer,
                        static_cast<int>(std::lround(alignedX(scene.profile, graphic))),
                        static_cast<int>(std::lround(graphic.y)));
  }

  if (!scene.text_runs.empty()) {
    visage::Screenshot text_layer =
        renderTextLayer(scene.profile, scene.text_runs, timeline.time_seconds);
    compositeSourceOver(result, text_layer, 0, 0);
  }

  return result;
}

void saveSceneFrame(const std::string& output_path, const Scene& scene, const Timeline& timeline) {
  savePngWithStraightAlpha(output_path, renderSceneFrame(scene, timeline));
}

} // namespace adt::composition
