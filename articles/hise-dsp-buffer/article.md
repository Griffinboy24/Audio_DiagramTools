A speaker makes sound by moving air.

If the cone moves forward, the air pressure rises. If it moves back, the air pressure falls. Do that fast enough and you hear it as sound. A waveform is a drawing of that movement over time.

![Speaker cone following a waveform](./renders/speaker-waveform.gif)

The top panel shows the waveform. The speaker below follows it. This first example is slow and tidy on purpose, so the relationship is easy to see.

Real audio is usually much messier than a tidy teaching wave:

![Voice sample driving a speaker](./renders/voice-sample-speaker.gif)

Even here, the idea is the same. The speaker follows a value that changes over time.

## Samples are points on the waveform

A computer does not store a smooth curve directly. It stores a series of measurements along the curve, and each measurement is called a **sample**.

Here is a tiny teaching waveform with only eight samples. The values are rounded so the table is readable:

![Sample numbers plotted as points](./renders/sample-array-to-plot.png)

Read the numbers from left to right and you get the shape of the waveform. Positive numbers move the speaker one way, negative numbers move it the other way, and zero is the centre position.

The table and the drawing are the same data. The table is the machine-friendly view; the drawing is the human-friendly view.

![Sample table and waveform playback](./renders/sample-table-playback.gif)

That is the first important idea: **digital audio is a stream of sample values.**

## Real audio has a lot of samples

The eight-sample example is only a teaching picture. Real audio is much denser. At a sample rate of 48 kHz, one second of mono audio contains 48,000 sample values.

![Dense samples continue beyond the page](./renders/dense-sample-waveform.png)

So when we talk about "the waveform" inside a computer, we are really talking about a long list of numbers changing very quickly.

## Buffers are short runs of samples

An audio effect in HISE does not usually process a whole sound file at once. The audio engine hands the effect a short run of samples, the effect processes that run, and then the next run arrives.

Those short runs are called **buffers**. In the next diagram I label them as chunks, because that is the most direct visual idea: a buffer is a chunk of the waveform.

![Waveform split into chunks](./renders/waveform-buffer-split.png)

The chunk edges are not musical. They do not care where the waveform starts, peaks, crosses zero, or repeats. They are just how the audio engine packages the work.

![Buffers moving through DSP code into the output stream](./renders/buffer-through-dsp.gif)

The moving-block version is the same idea shown as a process: a buffer arrives, the DSP code changes the numbers inside it, and the processed buffer joins the output stream. Then the next buffer arrives. That repeated handoff is the basic shape of real-time DSP.

## Gain is multiplication

In HISE, this can look like a Gain node:

![HISE gain node](./renders/hise-gain-node.png)

A gain node has controls around it, but the core DSP idea is small: multiply every sample by a gain value.

You probably already know the result by ear. Lower gain means quieter sound. On the waveform, that means the same shape becomes shorter.

![Waveform being scaled taller and smaller](./renders/waveform-volume-scale.gif)

If the gain is `0.5`, every sample becomes half as tall.

![Before and after multiplying samples by 0.5](./renders/sample-gain-comparison.png)

Nothing mysterious happened. The waveform kept the same shape, but all the sample values moved closer to zero.

As pseudocode, the heart of the node looks like this:

![HISE gain node pseudocode](./renders/hise-gain-node-code.png)

If `gain` is `1.0`, the samples stay the same. If `gain` is `0.5`, the waveform becomes half as tall. If `gain` is `0.0`, every sample becomes zero, which is silence.

That is DSP coding at its simplest: change the numbers, and the sound changes.

## Effects change buffers in different ways

Once you see a buffer as a list of numbers, audio effects become much less mysterious.

- Gain multiplies the numbers.
- Distortion bends the numbers.
- Delay stores numbers and plays them back later.
- An oscillator writes new numbers into the buffer instead of modifying existing ones.

Here is the oscillator version of the same block-processing picture. The incoming buffer is empty, the DSP node writes sine-wave samples into it, and the result becomes the output.

![Oscillator block factory writing sine samples](./renders/oscillator-block-factory.gif)

That is the foundation. Audio effects are little machines that receive numbers, change numbers, and send numbers onward.

The next part is where it gets more interesting:

If sound is only numbers, how do those numbers create tone, frequency, and spectrum?
