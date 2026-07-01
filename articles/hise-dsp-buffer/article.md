What does DSP code actually receive?

In HISE, it is not usually given an entire audio stream at once. It is called again and again with short runs of sample values.

The code changes or creates values for the current run, then the next run arrives.

That short run is a buffer.

The easiest way to make that concrete is to start at the speaker.

## Start at the speaker

A speaker makes sound by moving air. The cone moves forward and backward, pushing and pulling the air in front of it.

Draw that movement over time and you get a waveform.

![Speaker cone following a waveform](./renders/speaker-waveform.gif)

The diagram is slowed down so the motion is visible. When the signal moves up, the cone moves one way. When the signal moves down, it moves the other way.

Real audio moves much faster:

![Voice sample driving a speaker](./renders/voice-sample-speaker.gif)

The motion is too fast to follow frame by frame here, but the relationship is the same. The speaker follows the signal over time.

## The signal as samples

In digital audio, that changing signal is represented as a sequence of values.

Each value is one sample: the signal height at one moment in time.

A tiny example might look like this:

```text
[0.00,  0.70,  0.82,  0.27,  -0.51,  -0.86,  -0.51,  0.27]
```

Read the values from left to right and the waveform shape appears.

![Sample numbers plotted as points](./renders/sample-array-to-plot.png)

Values above zero sit above the centre line. Values below zero sit below it.

The table and the drawing are not two different things. They are two views of the same data.

![Sample table and waveform playback](./renders/sample-table-playback.gif)

The table is the code-friendly view. The drawing is the human-friendly view.

For DSP, the important point is that the samples are not labels for the waveform. They are the material the code works on.

## Real audio is dense

The example above has only eight values because it is a teaching diagram.

Real audio contains far more. At 48 kHz, one second of mono audio contains 48,000 sample values. Stereo contains two streams of that size.

![Dense samples continue beyond the page](./renders/dense-sample-waveform.png)

Nothing new has been introduced here. There are just many more samples, packed much more tightly in time.

## Buffers

An effect does not receive all of those samples at once.

During playback, HISE keeps calling the effect with short runs of consecutive samples. Each run is a buffer.

![Waveform split into chunks](./renders/waveform-buffer-split.png)

A buffer is a processing block, not a musical phrase.

Its edges do not have to line up with peaks, zero crossings, notes, transients, or waveform cycles. A buffer boundary is just where this block ends and the next block begins.

That is why the audio still has to be treated as one continuous stream. The blocks are separate only while the engine is handing them to the effect.

![Buffers moving through DSP code into the output stream](./renders/buffer-through-dsp.gif)

At this level, an effect has a simple shape:

1. receive a buffer of samples
2. change the samples
3. pass the buffer onward

Then the next buffer arrives.

## Gain as the first effect

Gain is a useful first example because it does not change the timing of the samples. It only changes their height.

In HISE, the effect might appear as a node with controls:

![HISE gain node](./renders/hise-gain-node.png)

At the sample level, lowering the volume means scaling the values toward zero.

![Waveform being scaled taller and smaller](./renders/waveform-volume-scale.gif)

If the gain is `0.5`, each sample becomes half as large.

![Before and after multiplying samples by 0.5](./renders/sample-gain-comparison.png)

Same order. Same shape. Smaller values.

In code, the centre of the operation is just a loop over the current buffer:

![HISE gain node pseudocode](./renders/hise-gain-node-code.png)

If `gain` is `1.0`, the samples stay where they are. If `gain` is `0.5`, the waveform is half as tall. If `gain` is `0.0`, every sample becomes zero, which is silence.

A real gain node also has controls, smoothing, parameter handling, and normal plugin behaviour around it. The core audio operation is still just multiplication.

## The pattern

This is the basic shape behind audio effects: an effect receives a buffer, changes the sample values inside it, and passes it on.

Gain multiplies the samples. Distortion bends or clips them. Delay stores samples and writes them back later.

An oscillator is the same idea from the other direction. Instead of modifying incoming samples, it writes new samples into the buffer.

![Oscillator block factory writing sine samples](./renders/oscillator-block-factory.gif)

That is the foundation for the next part: audio is a stream of sample values, buffers are the short blocks HISE gives to DSP code, and effects work by changing or creating those values.

Next we can ask what happens when those values repeat at different speeds, and why that creates pitch, tone, and spectrum.
