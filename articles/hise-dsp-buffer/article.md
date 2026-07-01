A speaker does not understand "sound".

It only moves.

Push the cone one way and the air pressure rises. Pull it the other way and the air pressure falls. Do that fast enough, and we hear it as sound.

![Speaker cone following a waveform](./renders/speaker-waveform.gif)

The line above is the instruction the speaker is following.

At each moment, the line says where the cone should be.

That line is a waveform.

Real audio is usually much messier than a tidy teaching wave:

![Voice sample driving a speaker](./renders/voice-sample-speaker.gif)

But the idea is the same. The speaker is following a value that changes over time.

---

## A waveform is stored as samples

A computer does not store a smooth line directly.

It stores measurements of the line.

Each measurement is called a **sample**.

Here is a tiny teaching waveform with only eight samples:

![Sample numbers plotted as points](./renders/sample-array-to-plot.png)

Read the numbers from left to right and you get the shape.

Positive numbers move the speaker one way. Negative numbers move it the other way. Zero is the centre position.

The table and the drawing are the same data. The table is the machine-friendly view. The drawing is the human-friendly view.

![Sample table and waveform playback](./renders/sample-table-playback.gif)

That is the first important idea:

**digital audio is a stream of numbers.**

---

## Real audio has a lot of samples

I am using tiny examples so the idea is visible.

Real audio is much denser.

At a sample rate of 48 kHz, one second of mono audio contains 48,000 sample values.

![Dense samples continue beyond the page](./renders/dense-sample-waveform.png)

So when we talk about "the waveform", we are really talking about a long list of numbers changing very quickly.

---

## HISE receives the samples in chunks

An audio effect in HISE does not usually receive the whole sound file at once.

It receives a short run of samples, processes those samples, and then receives the next short run.

Those short runs are called **buffers**.

![Waveform split into chunks](./renders/waveform-buffer-split.png)

A buffer is just a block of the waveform.

The block edges are not musical. They do not care where the waveform starts, peaks, crosses zero, or repeats. They are just how the audio engine packages the work.

![Buffers moving through DSP code into the output stream](./renders/buffer-through-dsp.gif)

The effect receives a buffer, changes the numbers inside it, and passes the buffer onward.

Then the next buffer arrives.

Then the next.

That repeated handoff is the basic shape of real-time DSP.

---

## The simplest effect is gain

In HISE, this can look like a Gain node:

![HISE gain node](./renders/hise-gain-node.png)

A gain node has controls around it, but the core DSP idea is very small:

**multiply every sample by a gain value.**

You probably already know the result by ear. Lower gain means quieter sound.

On the waveform, that means the same shape becomes shorter.

![Waveform being scaled taller and smaller](./renders/waveform-volume-scale.gif)

If the gain is `0.5`, every sample becomes half as tall.

![Before and after multiplying samples by 0.5](./renders/sample-gain-comparison.png)

Nothing mysterious happened.

The waveform did not become a different kind of sound. It kept the same shape, but all the sample values moved closer to zero.

As pseudocode, the heart of the node is this:

![HISE gain node pseudocode](./renders/hise-gain-node-code.png)

If `gain` is `1.0`, the samples stay the same.

If `gain` is `0.5`, the waveform becomes half as tall.

If `gain` is `0.0`, every sample becomes zero, which is silence.

That is DSP coding at its simplest: change the numbers, and the sound changes.

---

## Effects are different ways of changing the buffer

Once you see the buffer as a list of numbers, audio effects become much less magical.

Gain multiplies the numbers.

Distortion bends them.

Delay stores them and plays them back later.

An oscillator is the same idea turned around. Instead of receiving a waveform and modifying it, it writes a new waveform into the buffer.

![Oscillator block factory writing sine samples](./renders/oscillator-block-factory.gif)

Clear the buffer, write samples in the shape of a sine wave, and the output becomes a sine wave.

That is the foundation.

Audio effects are not made of mystery. They are little machines that receive numbers, change numbers, and send numbers onward.

The next part is where it gets more interesting:

if sound is only numbers, how do those numbers create tone, frequency, and spectrum?
