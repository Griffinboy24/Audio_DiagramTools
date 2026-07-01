Audio coding starts with one simple thing: a speaker makes sound by vibrating.

The speaker cone moves forward and backward, pushing and pulling the air in front of it. Draw that movement over time and you get a waveform.

For DSP, that movement has to be represented in a form code can process. In HISE, that means sample values, buffers, and effects that change those buffers before they continue onward.

## The waveform

In this slowed-down diagram, the speaker cone follows the shape of the waveform.

![Speaker cone following a waveform](./renders/speaker-waveform.gif)

When the signal moves up, the cone moves one way. When the signal moves down, it moves the other way. That movement pushes air, and we hear it as sound.

Real audio moves far faster than this teaching version:

![Voice sample driving a speaker](./renders/voice-sample-speaker.gif)

The GIF can only suggest the speed, but the relationship is the same: the speaker follows the changing signal over time.

## Samples

Inside digital audio, the waveform is represented as a row of numbers.

Those numbers are called **samples**.

A tiny example might look like this:

[0.00,  0.70,  0.82,  0.27,  -0.51,  -0.86,  -0.51,  0.27]

If you read those values from left to right, they describe the waveform shape.

![Sample numbers plotted as points](./renders/sample-array-to-plot.png)

Positive values sit above the centre line. Negative values sit below it. Zero is the centre.

The table and the drawing are two views of the same thing:

![Sample table and waveform playback](./renders/sample-table-playback.gif)

The table is useful for code. The drawing is useful for our eyes.

That is the first important idea: for DSP code, the sample values are the thing being processed. The drawing is a human-readable view of those values.

## Lots of samples

The example above is tiny because it needs to be readable.

Real audio contains far more samples. At a sample rate of 48 kHz, one second of mono audio contains 48,000 sample values. Stereo is two streams of that size.

![Dense samples continue beyond the page](./renders/dense-sample-waveform.png)

The principle is the same. There are just many more numbers.

## Buffers

Inside HISE, an effect does not usually receive a whole sound file at once.

It receives the sample stream in short chunks. Those chunks are called **buffers**.

![Waveform split into chunks](./renders/waveform-buffer-split.png)

A buffer is just a short block of the waveform data.

The buffer edges are not part of the sound. They do not have to line up with peaks, zero crossings, waveform cycles, notes, or transients. They are just the boundaries of the block currently being processed.

HISE processes one buffer through the effect chain, then the next buffer, then the next one. Later, those chunks join back into the continuous audio stream we hear.

![Buffers moving through DSP code into the output stream](./renders/buffer-through-dsp.gif)

So a DSP effect has this basic shape:

1. receive a buffer of samples
2. change the samples in some way
3. send the buffer onward

That is the core pattern.

## Gain

Gain is the simplest example of that pattern.

In HISE, the node might look like this:

![HISE gain node](./renders/hise-gain-node.png)

The control has a user interface around it, but the basic audio operation is small: volume is the height of the waveform.

![Waveform being scaled taller and smaller](./renders/waveform-volume-scale.gif)

To make the sound quieter, scale the waveform down.

Because the samples are the height values, scaling the waveform means multiplying the samples.

![Before and after multiplying samples by 0.5](./renders/sample-gain-comparison.png)

Same shape. Half the height.

In code, the centre of the operation is just a loop over the samples in the buffer:

![HISE gain node pseudocode](./renders/hise-gain-node-code.png)

If `gain` is `1.0`, the waveform stays the same. If `gain` is `0.5`, the waveform is half as tall. If `gain` is `0.0`, every sample becomes zero, which is silence.

A real HISE gain node has controls, smoothing, and normal plugin behaviour around it. But the central DSP idea is still simple: multiply the samples, and the volume changes.

## The basic shape

This is the basic idea behind audio effects.

An effect receives a buffer, changes the numbers inside it, and passes it on.

Gain multiplies samples. Distortion might bend or clip samples. Delay might store samples and play them back later.

An oscillator works from the other direction. If the buffer is empty and the DSP code writes new samples in the shape of a sine wave, the output is a sine wave.

![Oscillator block factory writing sine samples](./renders/oscillator-block-factory.gif)

That is enough for part one:

the audio signal is represented as sample values, buffers are short chunks of those values, and DSP code changes or creates those values.

The next question is where the subject starts to get more interesting:

if all we have is a row of numbers, how do those numbers create pitch, tone, and spectrum?
