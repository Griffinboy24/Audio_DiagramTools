This is Part 1 of a series on how DSP works inside HISE.

This post only does one job: make the data visible.

Audio coding starts with understanding one simple thing:

A speaker makes sound by vibrating.

![Speaker cone following a waveform](./renders/speaker-waveform.gif)

The line is the waveform.

The speaker cone follows that shape over time.

When the line moves up, the cone moves one way. When the line moves down, it moves the other way. That movement pushes and pulls air, and we hear it as sound.

Real audio is usually much faster and messier than a neat slow wave:

![Voice sample driving a speaker](./renders/voice-sample-speaker.gif)

It is not a neat repeating wave anymore.

The speaker is still following the shape. The animation is slowed down so the movement is visible.

So if we want to create or change sound, we need to create or change that shape.

## The shape as values

In digital audio, the waveform is represented as values over time.

A tiny example might look like this:

```text
[0.00,  0.70,  0.82,  0.27,  -0.51,  -0.86,  -0.51,  0.27]
```

Plot those values from left to right and the shape appears:

![Sample numbers plotted as points](./renders/sample-array-to-plot.png)

Each value is one **sample**: one height at one moment.

The order matters because the order is time.

![Sample table and waveform playback](./renders/sample-table-playback.gif)

The highlighted table cell and the highlighted point are the same moment in the sound.

The table is the data. The drawing is the same data made readable.

## Real audio is dense

The examples above are tiny so we can see every value.

Real audio is packed much more tightly. At 48 kHz, one second of mono audio contains 48,000 samples.

![Dense samples continue beyond the page](./renders/dense-sample-waveform.png)

That is still the same idea: values over time.

There are just far more of them.

## Buffers are short chunks

HISE does not send an effect the whole sound at once.

It works through short chunks of the stream:

![Waveform split into chunks](./renders/waveform-buffer-split.png)

Those chunks are called **buffers**.

The cut lines are not part of the sound. They do not have to line up with peaks, zero crossings, notes, or waveform cycles.

They are just processing boundaries.

![Buffers moving through DSP code into the output stream](./renders/buffer-through-dsp.gif)

One buffer enters the effect.

The effect changes the sample values.

The buffer joins the output stream.

Then the next buffer arrives.

That is the basic shape of an audio effect.

## Gain: the simplest change

A gain node is a good first example because the operation is easy to see.

![HISE gain node](./renders/hise-gain-node.png)

Volume is the height of the waveform.

![Waveform being scaled taller and smaller](./renders/waveform-volume-scale.gif)

To make the sound quieter, make the values smaller.

For example, multiply every sample by `0.5`:

![Before and after multiplying samples by 0.5](./renders/sample-gain-comparison.png)

Same shape.

Half the height.

Inside the buffer, the operation is just repeated across the samples:

![HISE gain node pseudocode](./renders/hise-gain-node-code.png)

In plain code shape:

```cpp
for each sample in the buffer
{
    sample = sample * gain;
}
```

If `gain` is `1.0`, the values stay the same.

If `gain` is `0.5`, the waveform is half as tall.

If `gain` is `0.0`, every value becomes zero, which is silence.

A real HISE gain node has controls, smoothing, and parameter handling around it. The audio operation at the centre is still multiplication.

## Effects and generators

This is the core idea behind audio effects:

an effect takes a buffer of sample values, changes those values, and passes the buffer onward.

Gain multiplies samples.

Distortion bends or clips samples.

Delay stores samples and writes them back later.

A generator is a little different. It can create samples instead of changing incoming ones.

For example, an oscillator can write values in the shape of a sine wave:

![Oscillator block factory writing sine samples](./renders/oscillator-block-factory.gif)

The table, the waveform, the buffer, and the code are all views of the same material:

sample values changing over time.

Next: how repeated values become pitch, tone, and spectrum.
