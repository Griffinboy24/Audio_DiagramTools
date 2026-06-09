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

## Griffinboy Waveform Reference

The canonical Griffinboy waveform style reference is committed at
`docs/style-reference/griffinboy-waveform-reference.png`.

Treat this as the default house look for waveform-forward diagrams unless a specific
article needs a different treatment. It establishes the thin rounded frame, the
dark framed content gradient, subtle technical grid, softly lit blue-white trace,
and waveform-underfill behavior.

Do not overwrite this reference image during experiments. Render new studies or
article graphics to new paths.

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
