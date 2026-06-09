# Audio Diagram Tools

Custom tools for producing consistent still and animated diagrams for audio science blog posts.

This repo is starting as a lean C++/Visage rendering pipeline: reusable visual tokens, deterministic PNG output, and a frame-sequence-to-GIF lane that can evolve as the house style settles.

## What Is Here

- `audio_diagram_tools`: a small reusable C++ library for drawing branded audio diagrams with Visage.
- `adt_render`: a CLI example that renders a volume-modulated sine wave as a PNG still or numbered PNG frames.
- `adt_style_lab`: a CLI for rendering structural style studies without baking them into the production example.
- `scripts/Build.ps1`: configure, build, and run the smoke test.
- `scripts/RenderExample.ps1`: build and render the first example.
- `scripts/EncodeGif.ps1`: build and render the first animated GIF through the C++ `cgif` encoder.
- `scripts/RenderStyleLab.ps1`: render the current set of visual style studies.
- `docs/visual-system.md`: first-pass visual conventions.
- `docs/pipeline.md`: rendering and GIF workflow.

## Quick Start

Visage is expected at `build/_vendor/visage` locally. That source is not committed because it is third-party/vendor state.

```powershell
.\scripts\Build.ps1
.\scripts\RenderExample.ps1 -Out artifacts/stills/am_sine.png
.\scripts\RenderExample.ps1 -Frames 120 -OutDir artifacts/frames/am_sine
.\scripts\EncodeGif.ps1 -Frames 120 -Out artifacts/gifs/am_sine.gif
.\scripts\RenderStyleLab.ps1 -OutDir artifacts/style_lab
```

The helper scripts use named canvas presets by default: `blog-wide` for stills/frame
sequences and `gif-preview` for GIFs. Use `-Preset`, `-Width`, or `-Height` when a
specific render needs to step outside those defaults.

`RenderStyleLab.ps1` defaults to `blog-banner` and renders studies that vary drawing
structure, not just palette. Treat those as taste probes, not final branding decisions.

If Visage lives elsewhere:

```powershell
.\scripts\Build.ps1 -VisageDir H:\path\to\visage
```

## Dependency Notes

GIF export is handled in-process by [cgif](https://github.com/dloebl/cgif), a small MIT-licensed C GIF encoder that supports RGB input, quantization, dithering, and animation size optimizations. It is fetched as a pinned CMake dependency and built with the project.

The local `Fonts/` folder is ignored because the current Diagraph Etc files say "Free for personal use only"; that is fine for local experimentation but should not be redistributed through a public GitHub repo.

## Project Shape

```text
src/audio_diagram_tools/   reusable render code
examples/                  command-line render entry points
scripts/                   local build/render/export helpers
tests/                     smoke tests for headless rendering
docs/                      visual and pipeline notes
```

Generated renders go under `artifacts/`, which is ignored.
