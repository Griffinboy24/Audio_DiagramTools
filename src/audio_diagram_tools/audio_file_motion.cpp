#include "audio_diagram_tools/audio_file_motion.h"

#include <algorithm>
#include <cmath>

namespace adt {
namespace {

constexpr float kPi = 3.14159265358979323846f;

float clamp01(float value) {
  return std::max(0.0f, std::min(1.0f, value));
}

float smoothstep(float edge0, float edge1, float value) {
  const float t = clamp01((value - edge0) / (edge1 - edge0));
  return t * t * (3.0f - 2.0f * t);
}

float section(float wrapped, float start, float end) {
  return std::clamp((wrapped - start) / (end - start), 0.0f, 1.0f);
}

float cubic(float y0, float m0, float y1, float m1, float u) {
  const float u2 = u * u;
  const float u3 = u2 * u;
  return (2.0f * u3 - 3.0f * u2 + 1.0f) * y0 +
         (u3 - 2.0f * u2 + u) * m0 +
         (-2.0f * u3 + 3.0f * u2) * y1 +
         (u3 - u2) * m1;
}

} // namespace

float audioFileWaveformValue(float progress) {
  const float wrapped = progress - std::floor(progress);
  float value = 0.0f;

  if (wrapped < 0.17f) {
    const float u = section(wrapped, 0.0f, 0.17f);
    const float wobble = 0.030f * std::sin(2.0f * kPi * (2.0f * u + 0.12f)) *
                         std::sin(kPi * u);
    value = 0.66f * std::sin(kPi * u) + wobble;
  }
  else if (wrapped < 0.335f) {
    const float u = section(wrapped, 0.17f, 0.335f);
    const float wobble = 0.028f * std::sin(2.0f * kPi * (1.65f * u + 0.34f)) *
                         std::sin(kPi * u);
    value = -0.70f * std::sin(kPi * u) + wobble;
  }
  else if (wrapped < 0.535f) {
    const float u = section(wrapped, 0.335f, 0.535f);
    const float amplitude = (0.62f + 0.24f * smoothstep(0.16f, 0.62f, u)) *
                            (1.0f - 0.12f * smoothstep(0.74f, 1.0f, u));
    const float phase = 0.55f * u + 1.10f * u * u + 5.85f * u * u * u;
    const float brightness = smoothstep(0.34f, 0.78f, u);
    value = amplitude * (0.90f * std::sin(2.0f * kPi * phase) +
                         0.10f * brightness *
                             std::sin(2.0f * kPi * (2.18f * phase + 0.13f)));
  }
  else if (wrapped < 0.558f) {
    const float u = section(wrapped, 0.535f, 0.558f);
    value = cubic(0.0f, -0.72f, -0.40f, -0.04f, u);
  }
  else if (wrapped < 0.586f) {
    const float u = section(wrapped, 0.558f, 0.586f);
    value = cubic(-0.40f, 0.12f, 0.20f, 0.08f, u);
  }
  else if (wrapped < 0.645f) {
    const float u = section(wrapped, 0.586f, 0.645f);
    const float trough = cubic(0.20f, -0.34f, -0.66f, 0.02f, u);
    const float contour = 0.055f * std::sin(2.0f * kPi * (1.35f * u + 0.16f)) *
                          (1.0f - smoothstep(0.55f, 1.0f, u));
    value = trough + contour;
  }
  else if (wrapped < 0.705f) {
    const float u = section(wrapped, 0.645f, 0.705f);
    value = cubic(-0.66f, 0.10f, 0.0f, 0.66f, u);
  }
  else {
    const float u = section(wrapped, 0.705f, 1.0f);
    const float settled = 0.60f * std::sin(2.0f * kPi * u);
    const float imperfect = 0.035f * std::sin(2.0f * kPi * (2.1f * u + 0.18f)) *
                            std::sin(kPi * u);
    value = settled + imperfect;
  }

  return std::clamp(value, -0.94f, 0.94f);
}

float voiceSampleWaveformValue(float progress) {
  if (progress <= 0.0f || progress >= 1.0f)
    return 0.0f;

  const float attack = smoothstep(0.0f, 0.024f, progress);
  const float release = 1.0f - smoothstep(0.72f, 1.0f, progress);
  const float decay = std::exp(-2.15f * progress);
  const float envelope = attack * release * (0.18f + 0.82f * decay);

  const float chirp_phase = 62.0f * progress - 18.0f * progress * progress;
  const float formant_phase = 103.0f * progress - 44.0f * progress * progress +
                              1.7f * std::sin(2.0f * kPi * progress);
  const float breath_phase = 151.0f * progress - 39.0f * progress * progress + 0.23f;
  const float body =
      0.58f * std::sin(2.0f * kPi * chirp_phase) +
      0.30f * std::sin(2.0f * kPi * formant_phase + 0.35f) +
      0.12f * std::sin(2.0f * kPi * breath_phase);

  const float consonant_burst =
      0.34f * std::sin(2.0f * kPi * (178.0f * progress + 0.19f)) *
      std::exp(-760.0f * (progress - 0.13f) * (progress - 0.13f));
  const float second_burst =
      0.24f * std::sin(2.0f * kPi * (142.0f * progress + 0.41f)) *
      std::exp(-560.0f * (progress - 0.42f) * (progress - 0.42f));

  return std::clamp(envelope * body + consonant_burst + second_burst, -0.96f, 0.96f);
}

float audioFilePlayheadProgress(const Timeline& timeline) {
  return static_cast<float>(timeline.normalized_time);
}

AudioFileSweep audioFileTwoStageSweep(const Timeline& timeline) {
  const float sweep = std::fmod(audioFilePlayheadProgress(timeline) * 2.0f, 2.0f);
  const bool erase_pass = sweep >= 1.0f;
  return { erase_pass ? sweep - 1.0f : sweep, erase_pass, true, true };
}

AudioFileSweep voiceSampleOneShotSweep(const Timeline& timeline) {
  const float phase = audioFilePlayheadProgress(timeline) -
                      std::floor(audioFilePlayheadProgress(timeline));
  constexpr float kHoldDuration = 0.165f;
  constexpr float kPassDuration = (1.0f - 2.0f * kHoldDuration) * 0.5f;
  constexpr float kFirstHoldStart = kPassDuration;
  constexpr float kFirstHoldEnd = kFirstHoldStart + kHoldDuration;
  constexpr float kSecondPassEnd = kFirstHoldEnd + kPassDuration;
  constexpr float kRightEdgeHold = 0.995f;

  if (phase < kFirstHoldStart)
    return { phase / kPassDuration, false, true, true };

  if (phase < kFirstHoldEnd)
    return { kRightEdgeHold, false, false, true };

  if (phase < kSecondPassEnd)
    return { (phase - kFirstHoldEnd) / kPassDuration, true, true, true };

  return { kRightEdgeHold, true, false, true };
}

float speakerDriveForAudioFileProgress(float progress, float gain) {
  return std::clamp(audioFileWaveformValue(progress) * gain, -1.0f, 1.0f);
}

float speakerDriveForVoiceSampleProgress(float progress, float gain) {
  return std::clamp(voiceSampleWaveformValue(progress) * gain, -1.0f, 1.0f);
}

} // namespace adt
