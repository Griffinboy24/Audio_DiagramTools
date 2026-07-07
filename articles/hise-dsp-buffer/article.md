DSP can seem like a black box to a beginner.

Sound goes in.

Code happens.

Different sound comes out.

But it all starts to make sense if you understand one simple thing:

A speaker makes sound by moving.

![Speaker cone following a waveform](./renders/speaker-waveform.gif)

A speaker makes sound by moving.

The line at the top is a waveform.

For now, think of it as the shape the speaker is following.

When the waveform moves up, the cone moves one way.

When the waveform moves down, the cone moves the other way.

That movement pushes air, and we hear it as sound.

The first graphic is simplified so the motion is easy to see.

Real audio is faster and messier, but the rule does not change:

the speaker is following a changing value over time.

![Voice sample driving a speaker](./renders/voice-sample-speaker.gif)

## The Waveform As Numbers

If we want to create a sound, we need to create a waveform.

In digital audio, the shape is stored as numbers.

A tiny piece of a waveform might look like this:

```text
[0.00,  0.70,  0.82,  0.27,  -0.51,  -0.86,  -0.51,  0.27]
```

Digital audio does not store the picture.

It stores the heights.

Plot those heights from left to right, and the waveform appears.

![Sample numbers plotted as points](./renders/sample-array-to-plot.png)

These height numbers are called samples.

Each number is one tiny sample of the waveform height at one moment in time.

The order matters because the order is time:

![Sample table and waveform playback](./renders/sample-table-playback.gif)

The array is the waveform data.

The drawing is the same data made easier to see.

## Lots Of Samples

The example above is tiny on purpose.

Real audio has far more samples than this.

I'm sure you've heard of sample rate.

Running a program at 48 kHz means that one second of mono audio contains 48,000 sample values.

That is the kind of resolution that produces smooth waves.

The simple drawings in this article are just readable versions.

Real audio is the same thing, packed more tightly in time.

![Dense samples continue beyond the page](./renders/dense-sample-waveform.png)

So far, we have looked at audio as one long strip of sample values.

But we have not talked about how a plugin generates or processes that stream.

## Chunks

Real-time DSP does not work through one big, long sound in one go.

It processes chunks of audio.

![Waveform split into chunks](./renders/waveform-buffer-split.png)

A DSP effect takes a short chunk of audio, processes it, then moves on to the next chunk.

Those chunks are called buffers.

An effect is like this:

A chunk comes in.

The effect changes the numbers inside it.

The chunk goes out.

Then the next chunk comes in.

![Buffers moving through DSP code into the output stream](./renders/buffer-through-dsp.gif)

This is the basic shape of a normal audio effect.

The reason we process in small chunks is partly about efficiency.

But it is also practical: some effects need access to a small section of audio at a time.

## A Simple Example: Volume

Let's look at a real-life example.

Volume is a good example because it is easy to see.

![HISE gain node](./renders/hise-gain-node.png)

You probably already know:

Volume is the height of the waveform.

Making a waveform less tall means the speaker is not moving as much.

Height affects volume.

![Waveform being scaled taller and smaller](./renders/waveform-volume-scale.gif)

Earlier, we saw that waveforms are stored as height values.

So all we need to do is make those sample values smaller.

For example, if we multiply every sample by `0.5`, the shape is the same, but the height is half as large.

![Before and after multiplying samples by 0.5](./renders/sample-gain-comparison.png)

The algorithm for a volume effect boils down to this:

![HISE gain node pseudocode](./renders/hise-gain-node-code.png)

In pseudocode form:

```cpp
receive buffer

for each sample in the buffer
{
    sample = sample * gain;
}

send buffer onward
```

Do the above for every buffer we receive.

If `gain` is `1.0`, the sample values stay the same.

If `gain` is `0.5`, the waveform is half as tall.

If `gain` is `0.0`, every sample becomes zero, which is silence.

In actual C++ DSP:

![HISE gain node C++ process function](./renders/hise-gain-node-cpp-code.png)

That is what actual DSP code looks like.

The main part of the effect is a function that receives a buffer of samples, which is our chunk, and processes them using math.

The real HISE gain node has extra controls and parameter smoothing, but the core audio operation is still this simple multiplication.

## The Lesson

Honestly, that is pretty much it.

Most audio effects work by doing this:

Take a buffer of sample values.

Change those values.

Pass the buffer onward.

```text
A gain effect multiplies the samples.
```

```text
A distortion effect bends the waveform shape by changing the samples.
```

```text
A delay effect stores samples and plays them back later.
```

Oscillators are slightly different.

They create new sample values rather than changing incoming ones.

![Oscillator block factory writing sine samples](./renders/oscillator-block-factory.gif)

If I am being completely honest, this is not quite the whole picture.

If we zoom out a little, it is really a loop: something is asking our DSP for samples, and we can give it whatever we want.

For an effect, something sends us a chunk and says: please process this.

We process it, then it takes the chunk back.

For an oscillator, we might be given an empty chunk, and we write a waveform into it.

Sometimes the chunk might already contain audio, for example other synth voices that were added earlier.

In that case, an oscillator would add its waveform on top instead of erasing the existing data.

That is getting ahead of this first article, though.

Some of these things are best learned by doing them.

## Recap

The table, the waveform, and the buffer are not separate ideas.

They are different views of the same thing:

sample values over time.

Audio plugins work by processing chunks of samples that are passed around between oscillators and effects.

![Chunks moving through oscillators and effects](./renders/plugin-chain-routing.gif)

Down the line, part of the program will mix those chunks and send them to the speaker.

That is when we hear the waveform.

We do not usually have to worry about that part. HISE or JUCE takes care of it for us.

![Output stream moving to a speaker](./renders/output-stream-to-speaker.gif)
