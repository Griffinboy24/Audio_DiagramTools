Hi HISE forum!

I'm starting a series on how DSP works in HISE, mainly for people who already build audio products but want a clearer route into C++ DSP nodes. This first part is deliberately basic: samples and buffers, the material an audio effect processes.

If you already know this part, skim it. I want the model pinned down before the later posts get into callbacks, custom nodes, and implementation details.

## The signal

A speaker makes sound by moving air. A waveform is a drawing of a changing audio signal over time. In a speaker, that changing signal becomes cone movement: positive values move one way, negative values move the other.

![Speaker cone following a waveform](./renders/speaker-waveform.gif)

That first graphic is slow on purpose. It is there to link the waveform shape to the speaker motion.

Real audio is denser and less tidy:

![Voice sample driving a speaker](./renders/voice-sample-speaker.gif)

Same relationship at higher density. The speaker is following a changing signal over time.

## Samples

For DSP code, that changing signal is represented as values taken at regular time intervals. Each value is a **sample**.

A tiny teaching example might look like this:

```text
[0.00,  0.70,  0.82,  0.27,  -0.51,  -0.86,  -0.51,  0.27]
```

The values are rounded so the example stays readable.

![Sample numbers plotted as points](./renders/sample-array-to-plot.png)

Follow the values from left to right and you get the waveform shape. Positive values are one side of the centre line, negative values are the other side, and zero is the centre.

The table and the drawing are the same data. The table is the code-friendly view; the drawing is the visual view.

![Sample table and waveform playback](./renders/sample-table-playback.gif)

That is the bit to keep: digital audio is a stream of sample values.

## Sample rate

The example above is tiny because it is a teaching diagram.

At a sample rate of 48 kHz, one second of mono audio contains 48,000 sample values. Stereo is two such streams.

![Dense samples continue beyond the page](./renders/dense-sample-waveform.png)

So the simplified drawings are not pretending real audio is small. They are zooming in enough that the idea is visible.

## Buffers

In a real-time effect, the audio stream is divided into short blocks before it reaches the processing code. Each short block is a **buffer**.

The diagram labels them as chunks, because that is the direct visual idea: a buffer is a chunk of the sample stream.

![Waveform split into chunks](./renders/waveform-buffer-split.png)

The chunk edges are scheduling boundaries, not musical boundaries. They do not need to line up with peaks, zero crossings, or waveform cycles.

![Buffers moving through DSP code into the output stream](./renders/buffer-through-dsp.gif)

The blocks are separate for processing, but the output is intended to be continuous.

At this level, an effect has a simple job:

1. receive a buffer of samples
2. change the samples in some way
3. pass the buffer onward

Then the next buffer arrives, and the same thing happens again.

## Gain

Gain is the smallest useful example. In HISE, this might be a Gain node:

![HISE gain node](./renders/hise-gain-node.png)

The node has controls around it, but the core DSP operation is simple: multiply the sample values.

![Waveform being scaled taller and smaller](./renders/waveform-volume-scale.gif)

If the gain is `0.5`, every sample becomes half as large.

![Before and after multiplying samples by 0.5](./renders/sample-gain-comparison.png)

Same shape. Half the height. At buffer level, volume is multiplication.

As pseudocode, the heart of the node looks like this:

![HISE gain node pseudocode](./renders/hise-gain-node-code.png)

If `gain` is `1.0`, the samples stay the same. If `gain` is `0.5`, the waveform is half as tall. If `gain` is `0.0`, every sample becomes zero, which is silence.

That is DSP coding at its simplest: change the sample values, and the output changes.

## The same pattern

Other effects use the same buffer-processing shape:

- gain multiplies samples
- distortion bends or clips samples
- delay stores samples and plays them back later

An oscillator starts from the other direction. Instead of changing incoming samples, it writes new sample values into the buffer.

Here is the oscillator version of the same block-processing picture. The incoming buffer is empty, the DSP node writes sine-wave samples into it, and the result becomes the output.

![Oscillator block factory writing sine samples](./renders/oscillator-block-factory.gif)

That is enough for part 1. The useful model is that audio is sample values over time, and a buffer is the short run of those values that DSP code processes before the next one arrives.

Next time we can ask the question this naturally leads to: how do those values create pitch, tone, and spectrum?
