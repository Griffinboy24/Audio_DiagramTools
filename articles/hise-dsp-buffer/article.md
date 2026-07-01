A gain knob is familiar: turn it down and the signal gets quieter.

At the DSP level, that control becomes a repeated operation:

take a short block of sample values, multiply each value, pass the block onward.

That short block is a **buffer**.

Most real-time audio effects in HISE are built around this pattern. To see why, start with the thing being processed: the waveform.

## The signal

A speaker makes sound by moving air. The audio signal is the changing value that drives that movement over time. Draw that value over time and you get a waveform.

![Speaker cone following a waveform](./renders/speaker-waveform.gif)

This diagram is slowed down so the relationship is visible: the line changes, and the speaker cone follows.

Real audio is denser than a teaching waveform:

![Voice sample driving a speaker](./renders/voice-sample-speaker.gif)

The motion is faster, but the relationship is the same. The speaker follows a changing signal over time.

## Samples

DSP code does not work with the drawing. It works with values.

The signal is represented by values taken at regular time intervals. Each value is a **sample**.

A tiny teaching example might look like this:

```text
[0.00,  0.70,  0.82,  0.27,  -0.51,  -0.86,  -0.51,  0.27]
```

The values are rounded so the table stays readable.

![Sample numbers plotted as points](./renders/sample-array-to-plot.png)

Read the values from left to right and the waveform shape appears. Positive values sit above the centre line, negative values sit below it, and zero is the centre.

The table and the drawing are the same data in two forms:

- the table is convenient for code
- the drawing is convenient for people

![Sample table and waveform playback](./renders/sample-table-playback.gif)

The practical point is that the drawing is not separate from the numbers. It is a way to see them.

Digital audio is a stream of sample values.

## Sample rate

The eight-value example above is deliberately tiny.

At 48 kHz, one second of mono audio contains 48,000 sample values. Stereo is two streams of that size.

![Dense samples continue beyond the page](./renders/dense-sample-waveform.png)

The diagrams in this article are zoomed in so the structure is visible. Real audio uses the same representation at a much higher density.

## Buffers

An effect does not usually process a whole sound file at once.

In real time, the audio stream is split into short blocks before it reaches the processing code. Each block is a **buffer**.

The next diagram labels them as chunks because that is the direct visual idea: a buffer is a chunk of the sample stream.

![Waveform split into chunks](./renders/waveform-buffer-split.png)

The chunk edges are not musical. They do not have to land on peaks, zero crossings, or waveform cycles. They are processing boundaries.

![Buffers moving through DSP code into the output stream](./renders/buffer-through-dsp.gif)

The blocks are separate while they are being processed, but the output has to join back into one continuous stream.

At this level, an effect is doing the same job over and over:

1. receive a buffer of samples
2. change the sample values
3. pass the buffer onward

Then the next buffer arrives.

## Gain

Now the gain knob from the start has somewhere to live.

In HISE, a gain effect might look like this:

![HISE gain node](./renders/hise-gain-node.png)

The node has controls and smoothing around it, but the core DSP operation is multiplication.

![Waveform being scaled taller and smaller](./renders/waveform-volume-scale.gif)

If the gain is `0.5`, every sample value is multiplied by `0.5`.

![Before and after multiplying samples by 0.5](./renders/sample-gain-comparison.png)

Same shape, half the height.

As pseudocode, the centre of the operation looks like this:

![HISE gain node pseudocode](./renders/hise-gain-node-code.png)

If `gain` is `1.0`, the samples stay the same. If `gain` is `0.5`, the waveform is half as tall. If `gain` is `0.0`, every sample becomes zero, which is silence.

That is the basic DSP move: change the sample values, and the output changes.

## The same pattern

Once the buffer model is clear, other effects fit the same shape:

- gain multiplies samples
- distortion bends or clips samples
- delay stores samples and plays them back later

An oscillator is the same pattern from the other direction. Instead of changing incoming samples, it writes new sample values into the buffer.

Here the incoming buffer is empty, the DSP node writes sine-wave samples into it, and the result becomes the output:

![Oscillator block factory writing sine samples](./renders/oscillator-block-factory.gif)

For this first part, that is the main idea:

audio is a stream of sample values, and a buffer is the short run of those values that DSP code receives, changes, and passes onward.

The next question is where things start to get more interesting:

how do those changing values create pitch, tone, and spectrum?
