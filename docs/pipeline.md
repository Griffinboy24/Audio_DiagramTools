# Rendering Pipeline

The pipeline is PNG-first:

1. Render a still PNG or a numbered PNG frame sequence with `adt_render`.
2. Inspect frames/stills in `artifacts/`.
3. Encode to GIF as a separate `gifski` export step.

That separation keeps the renderer deterministic and keeps export requirements explicit.

## Still Render

```powershell
.\scripts\RenderExample.ps1 -Out artifacts/stills/am_sine.png
```

Equivalent direct CLI:

```powershell
.\out\build\adt_render.exe --width 1280 --height 720 --out artifacts/stills/am_sine.png
```

## Frame Sequence

```powershell
.\scripts\RenderExample.ps1 -Frames 120 -Fps 30 -OutDir artifacts/frames/am_sine
```

This writes:

```text
artifacts/frames/am_sine/frame_0000.png
artifacts/frames/am_sine/frame_0001.png
...
```

## GIF Encoding

`gifski` is required. Missing encoder means setup failure.

```powershell
.\scripts\EncodeGif.ps1 -FramesDir artifacts/frames/am_sine -Output artifacts/gifs/am_sine.gif -Fps 30 -Width 960 -Quality 88
```

Why this setup:

- [gifski](https://gif.ski/) is open source, works from PNG frames, and focuses on high-quality color handling for GIF.
- One required encoder keeps output consistent and failures obvious.

For longer or large animations, we can add a separate video export lane later. That should be a first-class path with its own command and quality rules.
