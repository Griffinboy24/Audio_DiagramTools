# HISE DSP Buffer Renders

Final PNG/GIF exports for `articles/hise-dsp-buffer/article.md` live here.

Most files are generated from canonical graphics or HISE-width composite
graphics. The HISE gain node images are article-specific composites derived
from `../assets/hise-gain-node-source.png`.

## Canonical graphic exports

```powershell
out\build\adt_render_graphic.exe --graphic audio-file-to-speaker-scene --preferred-size --gif --frames 180 --fps 18 --out articles\hise-dsp-buffer\renders\speaker-waveform.gif
out\build\adt_render_graphic.exe --graphic voice-sample-to-speaker-scene --preferred-size --gif --frames 48 --fps 18 --out articles\hise-dsp-buffer\renders\voice-sample-speaker.gif
out\build\adt_render_graphic.exe --graphic dense-sample-waveform-graphic --preferred-size --out articles\hise-dsp-buffer\renders\dense-sample-waveform.png
out\build\adt_render_graphic.exe --graphic block-processing-graphic --preferred-size --gif --frames 150 --fps 15 --out articles\hise-dsp-buffer\renders\buffer-through-dsp.gif
out\build\adt_render_graphic.exe --graphic oscillator-block-factory-graphic --preferred-size --gif --frames 180 --fps 15 --out articles\hise-dsp-buffer\renders\oscillator-block-factory.gif
```

## Article composite exports

```powershell
out\build\adt_render_composite_graphic.exe --graphic sample-array-to-plot-article-scene --out articles\hise-dsp-buffer\renders\sample-array-to-plot.png
out\build\adt_render_composite_graphic.exe --graphic sample-table-playback-article-scene --gif --frames 8 --fps 2 --out articles\hise-dsp-buffer\renders\sample-table-playback.gif
out\build\adt_render_composite_graphic.exe --graphic waveform-buffer-split-article-scene --out articles\hise-dsp-buffer\renders\waveform-buffer-split.png
out\build\adt_render_composite_graphic.exe --graphic waveform-volume-scale-article-scene --gif --frames 120 --fps 25 --out articles\hise-dsp-buffer\renders\waveform-volume-scale.gif
out\build\adt_render_composite_graphic.exe --graphic sample-gain-comparison-scene --out articles\hise-dsp-buffer\renders\sample-gain-comparison.png
```

## Article-specific exports

- `hise-gain-node.png`
- `hise-gain-node-code.png`

These two images use the source screenshot in `../assets/` and should stay with
this article unless we promote the gain-node renderer into a reusable canonical
asset.
