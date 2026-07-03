# HISE DSP Buffer Article Research Brief

This file is the preparation step for the article rewrite. The article should
not be rewritten from taste alone; it needs a deliberate teaching shape.

## Reader

Cold HISE/forum reader. They may build instruments or effects already, but they
may not have written C++ DSP code.

They did not click because they want a formal definition of DSP. They clicked
because "How does DSP work?" promises that the hidden mechanism will become
visible.

The opening must reward that click quickly.

## Teaching Standard

The article should behave like a visual math/science explainer:

- Start with something the reader already understands or can see immediately.
- Make the reader curious before introducing technical names.
- Let diagrams carry part of the explanation.
- Add one truth layer at a time.
- Use short text to aim attention, not long text to brute-force comprehension.
- End each section with the reader feeling they noticed something themselves.

## Patterns From Research

### Audio-science app note style: context, mechanism, visualization, consequence

The Conformal BBD LFO note is the clearest style reference for this article. It
does not open with a generic tutorial voice. It starts from a real audio context,
names the specific mechanism under discussion, explains the mechanism in plain
technical prose, then places visualization at the exact point where the reader
needs to inspect a relationship.

Applied here:

- Open with the practical reason this matters: DSP code changes sample values,
  which eventually become speaker motion.
- Scope the note cleanly: Part 1 covers waveform, samples, buffers, and gain.
- Use real paragraphs, not a stack of isolated one-line claims.
- Use a graphic when the reader needs to see a relationship over time or between
  representations.
- After each graphic, state the consequence the reader should carry forward.
- Let equations/code arrive only after the visual model exists.

Source: Russell McClellan, "App Note 2: Modeling LFOs for BBD Chorus."
https://www.russellmcc.com/conformal/app_notes/2-bbd-lfo/

### Cold traffic needs an immediate promise, not a preamble

The reader arrives from the title. They are not asking for a definition of DSP,
and they are not asking for the author's motivation for making a series. They
want the hidden mechanism to become visible.

Applied here: the first paragraph should do only three jobs:

- say this is Part 1 of the series
- say the article will make samples and buffers visible
- move immediately into the first concrete example

The intro should not apologise for being basic, over-describe the audience, or
list everything that later posts will cover.

Sources studied:
https://www.redblobgames.com/grids/hexagons/
https://www.redblobgames.com/pathfinding/a-star/introduction.html
https://betterexplained.com/articles/an-interactive-guide-to-the-fourier-transform/

### Purpose before terminology

Nicky Case describes accessible explanations as Purpose, Intuition, then
Practice. The learner needs a reason to care before terminology appears.

Applied here: begin with sound coming from speaker movement, not with "DSP code
receives buffers." The buffer is a later name for a mechanism the reader has
already seen.

Source: Nicky Case, "Writing accessible explanations."
https://ncase.me/contact/

### Make the reader love the question

Nicky Case quotes Steven Strogatz on the failure mode of answering questions the
student has not thought to ask. The explanation should create the question in
the reader's mind first.

Applied here: show speaker motion following a waveform before explaining samples.
The question becomes "what is that shape made of?"

Source: Nicky Case, "How I Make Explorable Explanations."
https://blog.ncase.me/how-i-make-an-explorable-explanation/

### The first example must be concrete enough to trust

Bret Victor's ladder of abstraction is useful here because the article is not
just explaining a term; it is moving the reader between representations of the
same system.

Applied here:

- speaker and waveform: concrete motion over time
- array/table: symbolic data for the same motion
- plotted waveform: visual abstraction of the same data
- buffer chunks: system/engine view of the same data
- code loop: operation on the same data

If a section cannot say what representation it is adding, it is probably
unfocused.

Source: Bret Victor, "Up and Down the Ladder of Abstraction."
https://worrydream.com/LadderOfAbstraction/

### Climb the abstraction ladder slowly

Good explanations start grounded and move upward step by step.

Applied here:

1. speaker motion
2. waveform drawing
3. sample values
4. many samples
5. buffers/chunks
6. gain as multiplication
7. effects/generators as variations on changing or writing values

No section should skip two rungs at once.

Source: Nicky Case, "How I Make Explorable Explanations."
https://blog.ncase.me/how-i-make-an-explorable-explanation/

### Do and show and tell

Nicky Case's "Do & Show & Tell" pattern says text, graphs, animation, and
interaction have different jobs. Animation is best for temporal relationships;
graphs are best for broad relationships at a glance; text is best for abstract
concepts.

Applied here:

- Speaker GIF: show temporal following.
- Sample-table GIF: show ordered time steps.
- Dense sample image: show scale.
- Buffer split image: show chunk boundaries not matching waveform shape.
- DSP block GIF: show process order.
- Gain GIF/image: show transformation.
- Text should only name what the visual makes available.

Source: Nicky Case, "Explorable Explanations."
https://blog.ncase.me/explorable-explanations/

### Do not make visuals repeat the text

Distill's article on interactive articles frames web-native explanation as a
blend of text, static visuals, animation, simulation, and interaction. The point
is not to duplicate the same information in every mode; it is to use the medium
that carries each idea with the least friction.

Applied here:

- text should introduce what to attend to
- animation should carry motion/process
- still image should carry scale/spatial relation
- code should arrive only after the operation is visually obvious

Source: Distill, "Communicating with Interactive Articles."
https://distill.pub/2020/communicating-with-interactive-articles/

### Examples before formal definitions

BetterExplained argues that mathematical intuition starts from a natural
perspective before formal definitions. The formal view should feel like another
view of something already known.

Applied here: do not define "sample" before the reader has seen a waveform as a
shape. Do not define "buffer" before the reader has seen the stream split into
short chunks.

Sources:
https://betterexplained.com/articles/developing-your-intuition-for-math/
https://betterexplained.com/articles/intuition-isnt-optional/

### The formal statement should feel earned

BetterExplained's ADEPT structure is useful as a checklist: analogy, diagram,
example, plain-English version, then technical form. The exact order can vary,
but the technical form should not be the first thing the reader gets.

Applied here:

- For "sample": waveform visual first, then array/table, then name "sample."
- For "buffer": chunk visual first, then name "buffer."
- For "gain": scaling visual first, then multiplication.

Source: BetterExplained, "Learn Difficult Concepts with the ADEPT Method."
https://betterexplained.com/articles/adept-method/

### Reduce extraneous load

Mayer's multimedia learning work says people learn better when extraneous
material is removed, organization is cued, corresponding text and visuals are
near each other, and complex material is segmented.

Applied here:

- No long paragraphs before a figure.
- No diagram that merely repeats the previous paragraph.
- No extra caveats unless needed to prevent a wrong mental model.
- Keep each visual and its text in one local unit.
- Break the article into small learner-paced segments.

Sources:
https://www.cambridge.org/core/books/abs/cambridge-handbook-of-multimedia-learning/principles-for-reducing-extraneous-processing-in-multimedia-learning-coherence-signaling-redundancy-spatial-contiguity-and-temporal-contiguity-principles/CD5B7AE1279A9AB81F8EEBB53DBEC86E
https://www.cambridge.org/core/books/abs/cambridge-handbook-of-multimedia-learning/principles-for-managing-essential-processing-in-multimedia-learning-segmenting-pretraining-and-modality-principles/DD24C2F48B9B1277CE59F78276110258

### Progressive disclosure

Progressive disclosure initially shows only the important information, then
reveals advanced details when they become relevant.

Applied here:

- The first half should not explain oscillators, distortion, delay, spectrum, or
  frequency domain.
- Mention these only after the buffer/gain model is established.
- Keep "this is simplified" notes short and placed where they prevent a
  specific misconception.

Source: Nielsen Norman Group, "Progressive Disclosure."
https://www.nngroup.com/articles/progressive-disclosure/

### Visual-first technical articles use small local claims

Red Blob Games' A* introduction makes one claim, gives a visual/process, then
shows the tiny code loop. It starts with a visualization-friendly simplification
and openly says the simplification is for explanation.

Applied here: use simplified waveforms and tables openly, but keep them tied to
the real HISE mechanism. The gain loop should arrive only after the reader has
seen "same shape, half height."

Source: Red Blob Games, "Introduction to the A* Algorithm."
https://www.redblobgames.com/pathfinding/a-star/introduction.html

### Diagrams should be designed before paragraphs

Red Blob's notes on diagram structure separate input, algorithm, output, and
visualization. Even when our article is not interactive, every graphic should
have a job that can be described as a transformation or relation.

Applied here: each image/GIF should be treated as a scene with a single
instructional purpose, not as illustration after prose.

Source: Red Blob Games, "Diagram structure."
https://www.redblobgames.com/making-of/diagram-structure/

## Reference Technique Matrix

| Reference | Observed technique | Rule for this article |
| --- | --- | --- |
| Nicky Case: How I Make Explorable Explanations | creates curiosity before the lesson | make the reader want to know what the waveform is made of before saying "sample" |
| Nicky Case: Explorable Explanations | teaches isolated mechanics before combining them | speaker motion, samples, buffers, gain must each be isolated before the final DSP summary |
| Bret Victor: Ladder of Abstraction | moves between concrete and abstract views of the same system | every section must say "same thing, new representation" |
| BetterExplained: Fourier Transform | uses a concrete metaphor before formal machinery | do not lead with DSP terminology; lead with visible audio behavior |
| BetterExplained: ADEPT | analogy/diagram/example/plain-English/technical | code appears after visual and plain-English transformation |
| Red Blob A* | small visual simplification first, implementation later | simplified waveforms are allowed if their simplification is explicit and useful |
| Distill interactive articles | combines media types by affordance | no paragraph should explain what an animation can show better |
| Mayer multimedia learning | coherence, signaling, contiguity, segmentation | cut redundant caveats, keep labels/visuals close, one idea per visual unit |
| HISE Scriptnode docs | nodes process signal, audio usually in chunks of samples | article must preserve HISE's buffer/chunk truth without opening with it |
| EarLevel buffer note | buffer-full processing is a practical audio implementation pattern | buffer explanation should connect to processing order, not only storage |

## DSP/HISE Facts To Preserve

- A waveform can be represented as sample values over time.
- A sample is a value at one moment in time.
- At 48 kHz, mono audio has 48,000 samples per second.
- HISE/audio processing usually handles audio in buffers/chunks of multiple
  samples.
- HISE Scriptnode docs describe samplerate and buffer size as aspects of the
  signal, and processing as chunks of multiple samples.
- A normal effect can process an input buffer and output changed samples.
- A generator/oscillator does not need an incoming waveform; it can write new
  samples into an output buffer.
- Gain can be taught as multiplying samples by a gain value.

Sources:
https://docs.hise.dev/scriptnode/manual/glossary.html
https://docs.hise.dev/scriptnode/list/container/fix8_block.html
https://www.earlevel.com/main/2019/04/26/how-i-write-code/
https://www.izotope.com/community/blog/digital-audio-basics-sample-rate-and-bit-depth

## Mistakes To Avoid

- Do not open with "What does DSP code receive?"
- Do not open with a fake reader question.
- Do not explain a whole concept in text and then show the same thing again.
- Do not use diagrams as decoration after the explanation.
- Do not introduce buffers before the reader understands sample values.
- Do not introduce gain code before the reader has seen "same shape, smaller
  height."
- Do not make all DSP sound like it receives input; oscillators/generators are a
  later exception.
- Do not over-qualify early simplifications. Keep the correction layer nearby,
  but not before the simplified model has done its job.
- Do not use vague section titles like "The numbers."
- Do not use poetic abstraction such as "audio coding starts with the speaker."
- Do not write in a condescending "let me explain this mystery object" register.
- Do not use "what does DSP receive?" as a framing question; it is not a cold
  reader's natural question.
- Do not let headings become poetic labels. Headings should be plain signposts.
- Do not use the word "basically" to hide an imprecise claim.
- Do not call the waveform "automation" early even if that becomes a useful
  later mental model.
- Do not imply that the speaker literally receives the simplified drawn blocks;
  the blocks are an engine representation that later becomes continuous output.
- Do not make the article read like a slide deck. Short paragraphs are useful,
  but an article needs connective tissue and reasons for moving from one idea to
  the next.
- Do not reduce every post-figure sentence to "this is still the same idea";
  say the specific consequence.

## Article Pacing Contract

Each visual unit should follow this rhythm:

1. One short setup sentence.
2. The visual.
3. One or two short noticing sentences.
4. One new term or consequence.

The visual is not evidence after the paragraph. It is part of the sentence.

Example shape:

```text
The same shape can be stored as values:

[visual]

Read the values left to right.
The dots trace the waveform.
Each value is one sample.
```

This is deliberately sparse. The reader's attention should spend more time on
the figure than on the paragraph.

## Visual Unit Inventory

| Unit | Asset | Visual job | Text job | New term allowed |
| --- | --- | --- | --- | --- |
| 1 | speaker-waveform.gif | speaker follows a changing line | point attention at motion | waveform |
| 2 | voice-sample-speaker.gif | real audio is faster/messier | correct the simplified first example | none |
| 3 | sample-table-playback.gif | values and waveform advance together | name one value at one time | sample |
| 4 | dense-sample-waveform.png | density/scale | connect 48 kHz to many values | sample rate |
| 5 | waveform-buffer-split.png | chunks cut through a continuous waveform | chunk edges are processing boundaries | buffer |
| 6 | buffer-through-dsp.gif | chunks move through effect and rejoin stream | effect changes buffer then passes it on | DSP effect |
| 7 | hise-gain-node.png | HISE context, real object | say gain/volume is the example | gain |
| 8 | waveform-volume-scale.gif | same shape changes height | connect height to volume | amplitude |
| 9 | sample-gain-comparison.png | before/after values and shape | multiplication by 0.5 | multiply |
| 10 | hise-gain-node-code.png | real node plus small pseudo loop | bridge visual operation to code | for each sample |
| 11 | oscillator-block-factory.gif | generator writes new values into output | name oscillator as exception | oscillator |

## Draft Quality Gate

Before the article is allowed to replace `article.md`, it must pass these checks:

- The first screen must make the reader feel they opened the right article.
- No paragraph may introduce more than one new noun that needs explanation.
- Every heading must answer "where are we in the ladder?"
- Every image/GIF must be locally useful without reading a long paragraph first.
- Any technical correction must appear exactly where the simplified model would
  otherwise create the wrong idea.
- Code must appear only after the reader already understands the operation
  visually.
- The final summary must not pretend all DSP is one pattern; it must separate
  effects that transform incoming samples from generators that write samples.

## Article Shape To Try

### 1. Tiny orientation

One or two lines only:

- This is part 1.
- Goal: make the first DSP mechanism visible.

Then immediately:

"Audio coding starts with understanding one simple thing:
A speaker makes sound by vibrating."

### 2. Waveform as motion

Text should set up the speaker GIF, then name what the reader sees.

Do not over-explain amplitude yet.

### 3. Real audio is denser

Use the vocal sample GIF to correct the simplified first image. The text should
say the first graphic was slowed down and simplified; real audio is faster and
messier.

### 4. Shape becomes values

Use the array and table/waveform animation. The text should be just enough to
connect "row of numbers" to "shape over time."

Avoid the static sample plot unless it solves a visible gap. It may be removed if
the animated table does the job better.

### 5. Scale: 48k samples

Short note. The point is density, not sample-rate theory.

### 6. Buffers are chunks

Introduce the word only after the chunk diagram. Say chunks first, buffers
second.

### 7. HISE processes chunks

Use the block factory GIF. Text should describe the route: chunk enters effect,
changes, joins output stream.

### 8. Gain as first worked example

HISE gain node -> scaling GIF -> before/after table/image -> pseudocode.

The code arrives late. It should feel obvious.

### 9. Generalize carefully

Effects can multiply, bend, store, or write samples. Oscillator is the exception
that writes new values.

End by pointing to part 2: pitch/tone/spectrum.

## Rewrite Test

After drafting, every visual unit must pass:

1. What question does the previous sentence make the reader ask?
2. What does the image answer without words?
3. What one new term or truth does the next sentence add?
4. Did the paragraph explain something the visual should have carried?
5. Did the article move only one abstraction rung?
