If you use HISE and want to start writing your own C++ DSP, start with the thing every effect has to handle: audio is a stream of sample values, and your DSP code receives that stream in short blocks called buffers.

This first article is about that foundation. It starts with speaker movement and waveforms, then connects that to samples, buffers, and a simple gain effect.

Some of the audio theory here will be familiar. It is included because the later C++ posts need the same foundation.

## From motion to waveform

A speaker makes sound by moving air. When the cone moves forward and backward, it pushes and pulls the air in front of it. That movement is what we hear.

A waveform is a drawing of that movement over time.

![Speaker cone following a waveform](./renders/speaker-waveform.gif)

The first diagram is slowed down so the relationship is easy to see: the line changes, and the speaker follows that shape.

Real audio changes much faster:

![Voice sample driving a speaker](./renders/voice-sample-speaker.gif)

The GIF can only suggest that speed, but the rule is unchanged. The signal changes over time, and the speaker follows it.

## Samples

DSP code does not work with the drawing directly. Inside digital audio, the signal is represented as values at regular time positions.

Each value is a **sample**.

A tiny teaching example might look like this:

```text
[0.00,  0.70,  0.82,  0.27,  -0.51,  -0.86,  -0.51,  0.27]
```

Those numbers are deliberately rounded. They are here to make the structure readable, not to represent a full audio file.

Plot the same values from left to right and the waveform shape appears:

![Sample numbers plotted as points](./renders/sample-array-to-plot.png)

Positive values sit above the centre line. Negative values sit below it. Zero is the centre.

The table and the drawing are two views of the same data:

![Sample table and waveform playback](./renders/sample-table-playback.gif)

The table is the form that is convenient for code. The drawing is the form that is convenient for people.

## Real audio is dense

The eight-sample example is useful because you can see every value.

Real audio is much denser. At 48 kHz, one second of mono audio contains 48,000 sample values. Stereo contains two streams of that size.

![Dense samples continue beyond the page](./renders/dense-sample-waveform.png)

The representation has not changed. There are just many more values.

## Buffers

An effect does not usually receive a whole sound file in one go.

In real time, a HISE effect is called with short runs of consecutive samples. Each short run is a **buffer**.

The next graphic labels them as chunks because that is the visual idea: a buffer is a chunk of the sample stream.

![Waveform split into chunks](./renders/waveform-buffer-split.png)

The chunk boundaries are not part of the music. They do not have to line up with peaks, zero crossings, cycles, notes, or transients. They are processing boundaries.

A buffer can begin halfway through a waveform cycle. A change in the sound can stretch across more than one buffer. The output still has to join back into one continuous stream.

![Buffers moving through DSP code into the output stream](./renders/buffer-through-dsp.gif)

At this level, an effect is doing the same job again and again:

1. receive a buffer of samples
2. change the sample values
3. pass the buffer onward

Then the next buffer arrives.

## Gain

Gain is the simplest useful example because it changes only the height of the waveform.

In HISE, the effect might appear as a node with controls:

![HISE gain node](./renders/hise-gain-node.png)

At the sample level, the core operation is multiplication.

![Waveform being scaled taller and smaller](./renders/waveform-volume-scale.gif)

If the gain is `0.5`, every sample value is multiplied by `0.5`. Values above zero move closer to zero. Values below zero also move closer to zero.

![Before and after multiplying samples by 0.5](./renders/sample-gain-comparison.png)

Same shape, half the height.

In code, that idea becomes a loop over the samples in the buffer:

![HISE gain node pseudocode](./renders/hise-gain-node-code.png)

If `gain` is `1.0`, the buffer is unchanged. If `gain` is `0.5`, the waveform is half as tall. If `gain` is `0.0`, every sample becomes zero, which is silence.

That is the first practical DSP model: change the sample values, and the output signal changes.

## The same pattern

Other effects use the same buffer model.

Gain multiplies samples. Distortion bends or clips samples. Delay stores samples and writes them back later.

An oscillator is the same idea from the other direction. Instead of changing incoming samples, it writes new sample values into the buffer.

![Oscillator block factory writing sine samples](./renders/oscillator-block-factory.gif)

That is the foundation for the rest of the series: audio is a stream of sample values, and a buffer is the short run of those values that DSP code receives, changes, and passes onward.

Next we can ask what happens when those values repeat at different speeds, and why that gives us pitch, tone, and spectrum.
