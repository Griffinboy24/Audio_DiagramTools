# Visual System Notes

The visual system is deliberately token-first. We can change the taste later without rewriting every diagram.

## Canvas Sizes

Start with named presets rather than one-off pixel choices:

- `blog-wide`: `1280x720`, default blog/YouTube-style landscape diagrams.
- `blog-wide-large`: `1600x900`, higher-resolution source render for downsampling.
- `gif-preview`: `960x540`, compact GIF export when file size matters.
- `square`: `1080x1080`, square social previews.

Keep scene code resolution-independent: derive plot areas from margins and canvas size.
The source of truth is `adt::canvasPresets()` in `src/audio_diagram_tools/render_types.cpp`;
scripts should use those names unless a render genuinely needs custom dimensions.

## Current Palette Direction

The first pass is dark, high-contrast, and technical without becoming monochrome:

- Background: near-black charcoal with a restrained plum undertone.
- Primary waveform: cyan.
- Modulation/envelope: warm amber.
- Accent/glow: pink-red, used sparingly.
- Grid/axis: translucent white.

These live in `adt::Palette` in `src/audio_diagram_tools/render_types.h`.

## Griffinboy Waveform Graphic

The canonical Griffinboy waveform graphic is code, not an image. The reusable
component entry point is `adt::canonical::audioFilePlayerGraphic()` or the
waveform-specific scene functions when the article needs a standalone waveform.

Treat this as the default house look for waveform-forward diagrams unless a specific
article needs a different treatment. It establishes the thin rounded frame, the
dark framed content gradient, subtle technical grid, softly lit blue-white trace,
and waveform-underfill behavior.

Render previews to `artifacts/` during experiments, but keep the accepted code path
as the source of truth.

## Griffinboy Array Graphic

The canonical simple unlabelled array/table graphic is code, not an image. The
reusable component builder is `adt::canonical::arrayGraphic()` in
`src/audio_diagram_tools/canonical_components.h`.

Treat this as the default look for compact mathematical arrays when the graphic
should feel clean, direct, and paper-like rather than like the dark waveform
display world. It establishes the rounded pill silhouette, thick grey rules,
soft internal fill, black value text, and diffuse cast shadow.

Render previews to `artifacts/` during experiments, but keep the accepted code path
as the source of truth.

## Griffinboy Sample Values Plot Reference

The canonical labelled discrete sample-value plot reference is code, not an
image. The reusable component id is `sample-values-plot`.
Use `adt::canonical::sampleValuesPlot()` in new code.

Treat this as the default look for pale scientific plot cards where the reader
needs to inspect discrete values against an amplitude axis. It establishes the
soft card shadow, pale vertical-gradient surface, compact y-axis labels, rounded
dash guide rhythm, blue open sample markers, and value-label spacing.

Render previews to `artifacts/` during experiments, but keep the accepted code path
as the source of truth.

## Griffinboy Double Arrow Graphic

The canonical small flow arrow graphic is code, not an image. The reusable
component builder is `adt::canonical::doubleArrowGraphic()`; use
`DoubleArrowOptions::single_color`
for neutral article-flow arrows such as `0xff4e4e4e`.

Use this between related diagram elements when the transition should feel quiet
and instructional rather than like a large callout. It establishes the stacked
rounded chevron geometry and the two-tone periwinkle/blue hierarchy.

Render previews to `artifacts/` during experiments, but keep the accepted code path
as the source of truth.

## Griffinboy Audio File Player Graphic

The canonical audio file player graphic is code, not an image. The reusable
component builder is `adt::canonical::audioFilePlayerGraphic()`; its options
expose waveform drawing, playhead progress, and the erase/wipe pass.

Use this when an article needs a dark waveform-world container for file-player
or timeline-style explanations. It establishes the wide rounded player frame,
technical grid, progress trail, playhead/handle treatment, timestamp placement,
corner ticks, and subtle page shadow.

Render previews to `artifacts/` during experiments, but keep the accepted code path
as the source of truth.

The audio-file waveform and playhead timing model lives in
`src/audio_diagram_tools/audio_file_motion.h`. Use that module when another
component needs to react to the current file position. This keeps the player
waveform shape, two-stage wipe loop, and speaker drive mapping from drifting
apart.

The companion `adt::canonical::voiceSampleToSpeakerScene()` graphic uses the
same player/speaker composition, but swaps in a short decaying voice-sample
waveform. It plays left to right, briefly hides the playhead at the right edge,
then repeats the left-to-right pass while wiping the fill before a second
right-edge hold. This lets beginner-facing articles contrast the simplified
looping teaching waveform with a more realistic audio-rate snippet.

## Griffinboy Speaker Animation Graphic

The canonical speaker animation graphic with annotations is code, not an image.
The reusable component builder is `adt::canonical::speakerAnimationGraphic()`;
its options expose the cone drive, sound-wave drive, and caption visibility.

Use this as the reference for speaker-cone motion diagrams that need annotated
airflow/sound-wave marks and explanatory text. It establishes the clean technical
speaker illustration, animated cone/dust-cap motion, masking/occlusion behavior,
blue annotation arrows, dashed sound-wave marks, and centered explanatory caption
layout.

Render previews to `artifacts/` during experiments, but keep the accepted code path
as the source of truth.

## Canonical Components

Use `src/audio_diagram_tools/canonical_components.h` for accepted reusable
graphics. New C++ code should prefer component builders. The current CLI ids are:

- `array-graphic`
- `sample-values-plot`
- `double-arrow-graphic`
- `audio-file-player-graphic`
- `speaker-animation-graphic`
- `audio-file-to-speaker-scene`
- `voice-sample-to-speaker-scene`
- `hise-node-container`
- `block-processing-graphic`

New article graphics should target these component builders and option structs.
Old style-study ids are not part of the supported public workflow.

Components that are likely to be stacked into larger graphics expose
`clear_background` options. Keep that enabled for standalone previews, and turn
it off when a component is being placed by the composition layer onto a target
profile background.

Render accepted components directly with:

```powershell
out\build\adt_render_graphic.exe --list
out\build\adt_render_graphic.exe --graphic audio-file-to-speaker-scene --preferred-size --out artifacts\canonical\audio-file-to-speaker-scene.png
out\build\adt_render_graphic.exe --graphic audio-file-to-speaker-scene --preferred-size --gif --frames 180 --fps 18 --out artifacts\canonical\audio-file-to-speaker-scene.gif
```

## Composition Direction

The default article-composite model is a vertical stack inside a target profile:

- top buffer
- centered or manually offset component
- explicit spacer
- next component
- bottom buffer

This is deliberately closer to a layout recipe than to fixed slots. Slots become
useful only when a repeated format has proven itself. For now, profiles should
define canvas width, content width, default buffers, and standard spacer sizes,
while each composition remains free to tune left/right placement when the visual
weight demands it.

The HISE inline-compound working target is currently `920x642` source pixels with
an `800px` content band. The light background is `#ffffff`, matching the HISE
post image field. The dark-mode composition background is `#1d1d1d`, used when a
graphic should intentionally read as a dark block rather than disappear into the
forum page. Source images are expected to be scaled by the forum when embedded,
so the important invariant is consistent internal spacing and clear render-time
metadata, not matching the browser's final CSS pixel size.

Single-purpose dark graphics, such as the block-processing DSP animation, should
still enter this flow as canonical components. When the animation relies on
edge cropping or moving blocks, render the component at the full `920px` HISE
width with an opaque dark background that matches `hiseDarkArticleImageProfile()`;
then place it inside the profile so top/bottom padding stays controlled by the
composition layer.

The first code layer for this is `src/audio_diagram_tools/composition.h`. It is
intentionally small: a `Profile` describes the target canvas and content band,
then `verticalStack()` places canonical `Component` values by alignment, explicit
dimensions, y position, and optional x offset. Drop to `PlacedGraphic` only when
an article needs custom positioning that the stack recipe cannot express cleanly.

## Scene Conventions

- Separate data/math decisions from drawing decisions.
- Use explicit `Dimensions`, `Timeline`, `RenderStyle`, and scene-specific spec structs.
- Make animation loops deterministic by deriving phase from `Timeline::normalized_time`.
- Render stills and animation frames through the same code path.
- Prefer named scene functions over one-off scripts once a diagram may be reused.

## Branding Flexibility

Fonts are not wired into the first render core yet. The local Diagraph Etc files are useful for exploration, but their current readme marks them as personal-use-only, so they stay local and ignored. When the final blog font is chosen, add it as either:

- a properly licensed committed asset, or
- a local path/config option documented for private rendering.
