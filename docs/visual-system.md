# Visual System Notes

The visual system is deliberately token-first. We can change the taste later without rewriting every diagram.

## Canvas Sizes

Start with these targets:

- `1280x720`: default blog/YouTube-style landscape diagrams.
- `1600x900`: higher-resolution source render for downsampling.
- `1080x1080`: square social previews.
- `960x540`: compact GIF export when file size matters.

Keep scene code resolution-independent: derive plot areas from margins and canvas size.

## Current Palette Direction

The first pass is dark, high-contrast, and technical without becoming monochrome:

- Background: near-black charcoal with a restrained plum undertone.
- Primary waveform: cyan.
- Modulation/envelope: warm amber.
- Accent/glow: pink-red, used sparingly.
- Grid/axis: translucent white.

These live in `adt::Palette` in `src/audio_diagram_tools/render_types.h`.

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
