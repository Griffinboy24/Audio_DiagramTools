#pragma once

#include "audio_diagram_tools/canonical_components.h"
#include "audio_diagram_tools/render_types.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <visage/graphics.h>

namespace adt::composition {

inline constexpr uint32_t kHisePostBackground = 0xffffffff;
inline constexpr uint32_t kGriffinDarkBackground = 0xff1d1d1d;

enum class HorizontalAlign {
  Left,
  Center,
  Right,
};

struct Profile {
  std::string_view id;
  std::string_view description;
  Dimensions dimensions;
  float content_width = 800.0f;
  float top_padding = 24.0f;
  float bottom_padding = 24.0f;
  uint32_t background = 0xffffffff;
};

struct PlacedGraphic {
  std::string canonical_id;
  Dimensions dimensions;
  float y = 0.0f;
  HorizontalAlign align = HorizontalAlign::Center;
  float x_offset = 0.0f;
  std::optional<float> x;
  canonical::RenderOptions options;
};

struct StackItem {
  canonical::Component component;
  HorizontalAlign align = HorizontalAlign::Center;
  float x_offset = 0.0f;
  float gap_after = 0.0f;
};

struct TextRun {
  std::string label;
  float x = 0.0f;
  float y = 0.0f;
  float width = 0.0f;
  float height = 0.0f;
  float size = 18.0f;
  uint32_t color = 0xff000000;
  visage::Font::Justification justification = visage::Font::kTopLeft;
  bool faux_bold = false;
};

struct LabelBodyLineOptions {
  float label_width = 72.0f;
  float label_size = 23.0f;
  float body_size = 21.6f;
  float height = 32.0f;
  uint32_t label_color = 0xff4167d6;
  uint32_t body_color = 0xff171b24;
  bool label_faux_bold = true;
};

struct StackOptions {
  float top_padding = 24.0f;
  float default_gap = 24.0f;
  float bottom_padding = 24.0f;
  bool auto_height = true;
};

struct Scene {
  Profile profile;
  std::vector<PlacedGraphic> graphics;
  std::vector<TextRun> text_runs;
};

class StackBuilder {
public:
  explicit StackBuilder(Profile profile, StackOptions options = {});

  StackBuilder& add(const canonical::Component& component,
                    float gap_after = -1.0f,
                    HorizontalAlign align = HorizontalAlign::Center,
                    float x_offset = 0.0f);
  StackBuilder& addText(const TextRun& run);

  float cursorY() const;
  Scene build() const;

private:
  Scene scene_;
  StackOptions options_;
  float cursor_y_ = 0.0f;
  float last_gap_ = 0.0f;
  bool has_graphics_ = false;
};

Profile hiseInlineProfile(int height = 642);
Profile hiseArticleImageProfile(int height = 642);
Profile hiseDarkArticleImageProfile(int height = 642);

float contentLeft(const Profile& profile);
float contentCenterX(const Profile& profile);
Dimensions hiseArticleImageDimensions(int height = 642);

float alignedX(const Profile& profile, const PlacedGraphic& graphic);
PlacedGraphic place(const canonical::Component& component,
                    float y,
                    HorizontalAlign align = HorizontalAlign::Center,
                    float x_offset = 0.0f);
std::vector<TextRun> centeredLabelBodyLine(const Profile& profile,
                                           std::string label,
                                           std::string body,
                                           float y,
                                           float total_width,
                                           const LabelBodyLineOptions& options = {});
void addCenteredLabelBodyLine(Scene& scene,
                              std::string label,
                              std::string body,
                              float y,
                              float total_width,
                              const LabelBodyLineOptions& options = {});

Scene verticalStack(Profile profile,
                    const std::vector<StackItem>& items,
                    const StackOptions& options = {});

visage::Screenshot renderSceneFrame(const Scene& scene,
                                    const Timeline& timeline = {});

void saveSceneFrame(const std::string& output_path,
                    const Scene& scene,
                    const Timeline& timeline = {});

} // namespace adt::composition
