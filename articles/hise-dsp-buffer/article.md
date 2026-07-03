Most audio effects can be understood first as changes to a waveform over time.

A gain effect makes the waveform smaller or larger. A distortion effect bends it. A delay effect stores part of it and plays it back later. In HISE, those changes happen to sample values, usually in short chunks called buffers.

This first part builds that picture from the outside in: speaker motion, waveform shape, sample values, buffers, and one simple gain operation.

Audio coding starts with understanding one simple thing:

A speaker makes sound by vibrating.

![Speaker cone following a waveform](./renders/speaker-waveform.gif)

The waveform at the top is a drawing of motion over time. When the line moves up, the speaker cone moves one way; when the line moves down, the cone moves the other way. That movement pushes and pulls air, and we hear it as sound.

Real audio is usually not a neat slow wave, so here is the same idea with a more detailed sample:

![Voice sample driving a speaker](./renders/voice-sample-speaker.gif)

The speaker is still following a changing shape. The animation is slowed down so the motion is visible, but the relationship is the important part: change the waveform shape, and you change what the speaker does.

## Samples

Once the waveform is a shape over time, the digital version is easier to picture. A small piece of that shape can be represented as a row of values:

```text
[0.00,  0.70,  0.82,  0.27,  -0.51,  -0.86,  -0.51,  0.27]
```

If those values are plotted from left to right, the waveform appears:

![Sample numbers plotted as points](./renders/sample-array-to-plot.png)

Each value is one **sample**: one height at one moment in time. Positive values sit above the centre line, negative values sit below it, and zero is the centre.

The next graphic shows the same data in two forms at once. The highlighted table cell and the highlighted point are the same moment in the sound.

![Sample table and waveform playback](./renders/sample-table-playback.gif)

The table is not a separate concept from the waveform. It is the same information in a form the computer can process. The curve is the same information in a form we can see quickly.

## Real audio is dense

The row above is deliberately tiny. It is useful because every value can be read.

Real audio is much denser. At 48 kHz, one second of mono audio contains 48,000 samples:

![Dense samples continue beyond the page](./renders/dense-sample-waveform.png)

This does not change the idea. It only changes the scale. The waveform is still represented by values over time, but real audio contains far more of those values than a teaching diagram can show comfortably.

## Buffers

HISE does not process a whole song, note, or sample file in one enormous piece. It works through the stream in short chunks.

![Waveform split into chunks](./renders/waveform-buffer-split.png)

Those chunks are called **buffers**. The cut lines are not part of the sound, and they do not need to line up with peaks, zero crossings, notes, or waveform cycles. They are just processing boundaries.

An effect processes one buffer, then the next buffer, then the next one. Later, those chunks join back into the continuous output stream.

![Buffers moving through DSP code into the output stream](./renders/buffer-through-dsp.gif)

That gives us the basic shape of a normal audio effect: take the current buffer, change the sample values, and pass the buffer onward.

## Gain

A gain node is the simplest useful example because the operation is easy to see in the waveform.

![HISE gain node](./renders/hise-gain-node.png)

Volume is the height of the waveform. To make a sound quieter, make the values smaller.

![Waveform being scaled taller and smaller](./renders/waveform-volume-scale.gif)

For example, multiplying every sample by `0.5` keeps the same shape but makes every height half as large:

![Before and after multiplying samples by 0.5](./renders/sample-gain-comparison.png)

Inside the buffer, that operation is just repeated across the samples.

![HISE gain node pseudocode](./renders/hise-gain-node-code.png)

In code shape, the core idea is:

```cpp
for each sample in the buffer
{
    sample = sample * gain;
}
```

If `gain` is `1.0`, the values stay the same. If `gain` is `0.5`, the waveform is half as tall. If `gain` is `0.0`, every value becomes zero, which is silence.

A real HISE gain node has controls, smoothing, and parameter handling around it, but the audio operation at the centre is still multiplication.

## Effects and generators

At this level, many audio effects are variations on the same pattern.

Gain multiplies sample values. Distortion bends or clips them. Delay stores values and writes them back later. The details get much more interesting, but the starting point is still a buffer of samples being changed over time.

Generators are slightly different. An oscillator does not need to reshape an incoming waveform; it can write new sample values into an output buffer.

![Oscillator block factory writing sine samples](./renders/oscillator-block-factory.gif)

That is why the table, the waveform, the buffer, and the code are useful views of the same thing: sample values changing over time.

Next: how repeated values become pitch, tone, and spectrum.
