Audio coding starts with the speaker.

A speaker makes sound by moving air. It moves forward, it moves back, and that movement creates changes in air pressure.

Draw the movement over time and you get a waveform.

![Speaker cone following a waveform](./renders/speaker-waveform.gif)

When the signal moves up, the speaker moves one way. When the signal moves down, it moves the other way.

Real audio moves much faster:

![Voice sample driving a speaker](./renders/voice-sample-speaker.gif)

The motion is too fast to follow frame by frame, but the idea is the same. A changing signal produces changing speaker movement.

## The numbers

In digital audio, we represent that changing signal as numbers.

Each number is a **sample**: the height of the signal at one moment in time.

[0.00,  0.70,  0.82,  0.27,  -0.51,  -0.86,  -0.51,  0.27]

Read the numbers from left to right and the waveform shape appears.

![Sample numbers plotted as points](./renders/sample-array-to-plot.png)

Positive numbers sit above the centre line. Negative numbers sit below it. Zero is the centre.

The table and the drawing are the same data in two forms:

![Sample table and waveform playback](./renders/sample-table-playback.gif)

The table is useful for code. The drawing is useful for us.

## The stream

The example above is tiny so we can see every value.

Real audio is dense. At 48 kHz, one second of mono audio contains 48,000 sample values. Stereo has two streams of that size.

![Dense samples continue beyond the page](./renders/dense-sample-waveform.png)

Nothing has changed conceptually. There are just many more samples, packed much more tightly in time.

## The buffer

An effect does not process the whole stream in one go.

It works in short blocks.

Those blocks are called **buffers**.

![Waveform split into chunks](./renders/waveform-buffer-split.png)

A buffer is a short block of sample values.

The block edges are not part of the sound. They do not have to line up with peaks, zero crossings, notes, transients, or waveform cycles.

They are just processing boundaries.

![Buffers moving through DSP code into the output stream](./renders/buffer-through-dsp.gif)

For a normal audio effect, the shape is:

1. take the current buffer
2. change the sample values
3. pass the buffer onward

Then the next buffer comes through.

## Gain

Gain is the cleanest example.

In HISE, a gain node might look like this:

![HISE gain node](./renders/hise-gain-node.png)

Volume is the height of the waveform. To make the sound quieter, make the numbers smaller.

![Waveform being scaled taller and smaller](./renders/waveform-volume-scale.gif)

Multiply every sample by 0.5, and the waveform is half as tall.

![Before and after multiplying samples by 0.5](./renders/sample-gain-comparison.png)

Same order. Same shape. Smaller values.

The core operation is just a loop over the buffer:

![HISE gain node pseudocode](./renders/hise-gain-node-code.png)

If gain is 1.0, the samples stay the same. If gain is 0.5, the waveform is half as tall. If gain is 0.0, every sample becomes zero, which is silence.

A real gain node has controls, smoothing, and parameter handling around it. The audio idea is still multiplication.

## Effects and generators

That is the basic shape of an audio effect:

take a buffer, change the numbers, pass it on.

Different effects change the numbers in different ways. Gain multiplies them. Distortion bends or clips them. Delay stores them and writes them back later.

An oscillator is slightly different. It does not need an incoming waveform. It can fill an empty buffer with new sample values.

![Oscillator block factory writing sine samples](./renders/oscillator-block-factory.gif)

That is enough for part one.

Audio is represented as sample values. A buffer is a short block of those values. DSP code works by changing them, storing them, or writing new ones.

Next: how repeating numbers become pitch, tone, and spectrum.
