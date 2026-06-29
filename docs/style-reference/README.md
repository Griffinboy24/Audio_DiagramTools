# Griffinboy Style Code References

Canonical style references in this project are Visage/C++ render paths, not saved
PNGs. Generated PNGs are previews only and belong under `artifacts/`.

## Accepted Code References

| Reference | Code Entry Point | What It Establishes |
| --- | --- | --- |
| Griffinboy waveform graphic | canonical sample waveform renderer in `src/audio_diagram_tools/sampled_waveform_scene.cpp` | Canonical dark waveform frame with smooth trace, zero axis, blue underfill, sample dots, technical grid, and corner ticks. |
| Griffinboy array graphic | `canonical::arrayGraphic()` in `src/audio_diagram_tools/canonical_components.h` | Simple unlabelled array treatment, rounded pill geometry, separator weight, internal fill, text tone, and soft cast shadow. |
| Griffinboy sample values plot | `canonical::sampleValuesPlot()` in `src/audio_diagram_tools/canonical_components.h` | Pale plotted sample-value card, soft shadow, compact axis labels, rounded dash guides, blue open sample markers, and value-label spacing. |
| Griffinboy double arrow graphic | `canonical::doubleArrowGraphic()` in `src/audio_diagram_tools/canonical_components.h` | Small stacked chevron flow arrow with rounded stroke terminals; color can be overridden for compound scenes. |
| Griffinboy audio file player graphic | `canonical::audioFilePlayerGraphic()` in `src/audio_diagram_tools/canonical_components.h`; motion model in `src/audio_diagram_tools/audio_file_motion.cpp` | Dark waveform-style file-player frame, technical grid, progress trail, playhead treatment, timestamps, corner ticks, and subtle cast shadow. |
| Griffinboy speaker animation graphic | `canonical::speakerAnimationGraphic()` in `src/audio_diagram_tools/canonical_components.h` | Procedural speaker-cone animation with clean technical geometry, moving masked dust cap, sound-wave marks, and centered explanatory annotation text. |
| Audio file to speaker scene | `canonical::audioFileToSpeakerScene()` in `src/audio_diagram_tools/canonical_components.h` | HISE-width compound scene joining file-player motion, double arrow, speaker animation, and explanatory text. |
| HISE node container | `canonical::hiseNodeContainer()` in `src/audio_diagram_tools/canonical_components.h` | Dark HISE-style node/container treatment with layered panel rectangles, top-heavy shadow gradient, HISE-style label font, power glyph, and close glyph. |

## Rules

- Do not promote PNG files as canonical references.
- Render previews to `artifacts/` when judging visual output.
- Promote a style only by keeping the accepted Visage code path and documenting its render entry point here.
- If an accepted style changes, update the code and this index together.
