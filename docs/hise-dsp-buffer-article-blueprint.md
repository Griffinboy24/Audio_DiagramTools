# HISE DSP Buffer Article Blueprint

This is the article architecture before touching the published markdown again.
It exists to keep the prose from drifting into generic AI tutorial voice.

## Reader State

The reader clicked a HISE forum/blog post called "How does DSP work?" They may
already make audio products. They may know what a waveform looks like. They may
not have a clean mental model for what C++ DSP code actually manipulates.

They need a first mechanism, not a complete theory of audio.

## Promise

By the end of Part 1, the reader should be able to say:

```text
Audio is sample values over time.
HISE processes those values in short chunks called buffers.
A simple effect changes the values and passes the chunk onward.
Gain is the easy example: multiply the values.
```

That is enough. Frequency, filters, spectrum, phase, aliasing, and plugin
architecture are later posts.

## Tone Rules

- Plain, precise, calm.
- Short paragraphs.
- No fake mystery framing.
- No "if you use HISE..." opener.
- No patronising "not a mysterious object" constructions.
- No poetic headings.
- No apology for basics.
- Do not over-address the reader; teach through the sequence.

## Section Sequence

### 1. Orientation

Job: Tell cold traffic what this post is and immediately reward the click.

Draft shape:

```text
This is Part 1 of a series on writing DSP in HISE.

The first step is not filters, FFTs, or C++ templates.
The first step is seeing what audio code changes.
```

Then immediately move to the speaker line:

```text
Audio coding starts with understanding one simple thing:
A speaker makes sound by vibrating.
```

Reason: This preserves the strong line from the original draft but gives it a
small landing pad so it does not feel like the wrong page.

### 2. Waveform As Motion

Asset: `speaker-waveform.gif`

Setup: one sentence only.

After visual:

- the line is the waveform
- the cone follows it over time
- air moves, sound happens

Do not introduce samples here.

### 3. Real Audio Is Faster And Messier

Asset: `voice-sample-speaker.gif`

Job: Correct the simplified first graphic without derailing the article.

Text should say:

- real audio is not usually a neat slow wave
- the relationship is still the same
- the animation is slowed down so the motion can be seen

Do not teach sample rate yet.

### 4. The Shape As Values

Possible assets:

- `sample-array-to-plot.png`
- `sample-table-playback.gif`

Job: Move one rung up: visible motion becomes stored values.

Preferred order:

1. show the compact array in prose
2. show plotted samples if it helps the "values become shape" moment
3. show table playback to add time/order

Keep the array values identical to the approved graphic:

```text
[0.00,  0.70,  0.82,  0.27,  -0.51,  -0.86,  -0.51,  0.27]
```

Term: `sample`

Critical wording:

```text
Each number is one sample: one value at one moment.
```

Do not say "a waveform becomes numbers." That implies an unnecessary conversion
story. Say digital audio stores/represents the shape as values.

### 5. Dense Real Audio

Asset: `dense-sample-waveform.png`

Job: Correct the tiny teaching array.

Text:

- the tiny array is only readable because it is tiny
- 48 kHz means 48,000 samples per second
- the idea is the same, just densely packed

Do not add bit depth, Nyquist, stereo detail, or sample-rate debate.

### 6. Chunks Become Buffers

Asset: `waveform-buffer-split.png`

Job: Introduce buffer by first showing chunks.

Preferred sequence:

```text
HISE does not hand an effect the whole song at once.
It works through short chunks of the stream.

[visual]

Those chunks are called buffers.
The cut lines are not part of the sound.
```

This is the place to prevent the misconception that chunks line up with cycles.

### 7. Through The DSP Node

Asset: `buffer-through-dsp.gif`

Job: Show process order.

Text should be smaller than the visual:

```text
One buffer enters the effect.
The effect changes the values.
The buffer joins the output stream.
Then the next buffer arrives.
```

Do not over-explain engine scheduling.

### 8. Gain

Assets:

- `hise-gain-node.png`
- `waveform-volume-scale.gif`
- `sample-gain-comparison.png`
- `hise-gain-node-code.png`

Job: First worked example.

Order:

1. HISE gain node as real anchor.
2. Scaling GIF: volume is height.
3. Before/after values/shape: multiply by 0.5.
4. Pseudocode: the operation in code.

The code should feel inevitable, not surprising.

### 9. Effects And Generators

Asset: `oscillator-block-factory.gif`

Job: Generalize without lying.

Text:

- effects usually transform incoming samples
- gain multiplies
- distortion bends/clips
- delay stores and reuses
- generators/oscillators can write new samples instead

End by pointing to next post: how repeated values become pitch/tone/spectrum.

## Visual Rhythm

Use this repeated unit:

```text
Short setup.

![visual](...)

One noticing sentence.
One naming sentence.
```

Break the rhythm only for code, where a short paragraph can introduce the
operation first.

## Hard Rejection Tests

Reject a draft if:

- a section begins with an abstract question nobody asked
- a paragraph explains a full mechanism before the diagram
- the same idea is stated three ways in a row
- a visual could be removed without losing meaning
- headings sound like chapter titles from a novel
- the first three paragraphs do not clearly belong under "How does DSP work?"
- the article says "DSP receives" before the reader has seen samples/chunks
- oscillator language implies every DSP node needs input audio

