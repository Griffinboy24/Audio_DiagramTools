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

`RenderExample.ps1` uses the `blog-wide` canvas preset by default.

Equivalent direct CLI:

```powershell
.\out\build\adt_render.exe --preset blog-wide --out artifacts/stills/am_sine.png
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
.\scripts\EncodeGif.ps1 -Frames 120 -Fps 30 -Out artifacts/gifs/am_sine.gif
```

`EncodeGif.ps1` uses the `gif-preview` canvas preset by default. Pass `-Preset blog-wide`,
or pass explicit `-Width`/`-Height` values, when a different target is needed.

Available presets can be listed from either CLI:

```powershell
.\out\build\adt_render.exe --list-presets
.\out\build\adt_encode_gif.exe --list-presets
```

Why this setup:

- [cgif](https://github.com/dloebl/cgif) is C99/MIT and designed as an actual encoder library.
- Its RGB path handles true-color input with quantization and dithering.
- Animation output stays scriptable and reproducible from our own render code.
- New animated scenes can use `adt::saveAnimatedGif(...)` by providing a frame-rendering callback.

For longer or large animations, we can add a separate video export lane later. That should be a first-class path with its own command and quality rules.
