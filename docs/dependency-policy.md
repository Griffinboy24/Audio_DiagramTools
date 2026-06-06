# Dependency Policy

This project should stay easy to clone and reason about.

## Committed

- Project source code.
- Small scripts and docs.
- Assets with clear redistribution permission.

## Local-Only

- `build/_vendor/visage`: local third-party checkout. Point CMake at it with `AUDIO_DIAGRAM_VISAGE_DIR`.
- `REFERENCES/`: research/reference code, including chowdsp_utils.
- `Fonts/`: current local font experiments are personal-use-only and should not be pushed.
- `artifacts/`: generated frames, stills, GIFs, videos.

## Tooling Choices

Keep the C++ core focused on deterministic rendering. Use external command-line tools for container/encoding work unless we later need a tightly integrated renderer/exporter.

Current export path:

1. PNG stills/frame sequences from Visage.
2. `cgif` for animated GIFs, built as a pinned CMake dependency.

Add future exporters as explicit tools with their own documented commands.

## Pinned Third-Party Code

- `cgif`: `https://github.com/dloebl/cgif.git`, commit `f1272ac2b0150e6f699163d70421b0347b72ec8e`, MIT license.
