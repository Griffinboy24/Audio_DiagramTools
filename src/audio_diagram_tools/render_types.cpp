#include "audio_diagram_tools/render_types.h"

#include <algorithm>

namespace adt {

Timeline Timeline::forFrame(int frame_index, int frame_count, double fps) {
  Timeline timeline;
  timeline.frame_count = std::max(1, frame_count);
  timeline.frame_index = std::max(0, std::min(frame_index, timeline.frame_count - 1));
  timeline.fps = fps > 0.0 ? fps : 30.0;
  timeline.time_seconds = static_cast<double>(timeline.frame_index) / timeline.fps;
  timeline.normalized_time = static_cast<double>(timeline.frame_index) /
                             static_cast<double>(timeline.frame_count);
  return timeline;
}

} // namespace adt
