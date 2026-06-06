# Rendering Pipeline

The pipeline is PNG-first:

1. Render a still PNG or a numbered PNG frame sequence with `adt_render`.
2. Inspect frames/stills in `artifacts/`.
3. Render animated GIFs with `adt_encode_gif`.

That separation keeps still-image iteration cheap while making animation export a first-class project tool.

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

GIF export is compiled into the project via `cgif`; no external GUI or PATH tool is required.

```powershell
.\scripts\EncodeGif.ps1 -Frames 120 -Fps 30 -Width 960 -Height 540 -Out artifacts/gifs/am_sine.gif
```

Why this setup:

- [cgif](https://github.com/dloebl/cgif) is C99/MIT and designed as an actual encoder library.
- Its RGB path handles true-color input with quantization and dithering.
- Animation output stays scriptable and reproducible from our own render code.

For longer or large animations, we can add a separate video export lane later. That should be a first-class path with its own command and quality rules.
