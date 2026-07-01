# Preview Lab

The preview lab emulates publishing surfaces for generated diagram assets. It is
separate from the Visage renderers: render PNG/GIF assets into `artifacts/`, then
inspect them in a target page context.

## HISE Published Topic

For normal article work, use the root dashboard:

```powershell
.\preview.ps1 -Open
```

Then open:

```text
http://127.0.0.1:8066/preview.html
```

The dashboard reads `articles/index.json` and links to each article in reader
mode and lab-tools mode.

For direct project-backed previews, serve the repository root first:

```powershell
python scripts/serve_preview_lab.py 8066
```

Then open:

```text
http://127.0.0.1:8066/preview_lab/hise-published-topic.html?project=../articles/hise-dsp-buffer/article.json&tools=1
```

Opening the HTML file directly still shows a built-in fallback article, but
browsers often block local JSON/Markdown loading from `file://`, so the local
server is the normal development route.

This profile is based on live measurements from published HISE forum blog
topics:

- responsive container breakpoints: Bootstrap-style `540 / 720 / 960 / 1140 / 1320px`
- desktop post content column: `850px`
- uploaded images: `max-width: 85%`
- body text: Inter, `14.5px / 21.75px`
- paragraph bottom spacing: `16px`
- page chrome: dark HISE forum top bar, topic header, author rail, post column
- markdown specimen styles for headings, lists, emphasis, code blocks, inline code,
  links, and blockquotes
- local copy of the HISE site logo; personal avatars are neutral placeholders
  unless a real asset is deliberately supplied

The lab includes debug controls for:

- loading an article project JSON file
- overriding the first figure with a single asset path for quick checks
- showing image natural size, displayed size, and scale factor for all figures
- toggling layout guides
- switching between logged-in and guest forum chrome
- showing a markdown styling specimen
- switching image background to reveal transparent-edge seams or preview the
  Griffin dark target background (`#1d1d1d`)
- collapsing the lab panel when you want an uncluttered reader-style view

By default, the page opens in a clean published-topic view: the lab panel is
collapsed and the scale report is hidden. Use `tools=1` to expand the lab.

## Article Projects

Article projects live under `articles/<article-name>/` and are intentionally
plain, file-driven folders. The root dashboard is the project picker. The lab
itself still loads one project JSON file at a time.

```text
articles/hise-dsp-buffer/
  article.json
  article.md
  assets/
  renders/
```

The JSON file stores forum-shell metadata and points to the article markdown:

```json
{
  "schema": "audio-diagram-tools.article-project.v1",
  "targetProfile": "hise-published-topic",
  "topicTitle": "[Blog] My Favourite C++ Open Source DSP References",
  "author": "griffinboy",
  "age": "22 days ago",
  "category": "Blog Entries",
  "posts": "10 posts",
  "posters": "5 posters",
  "views": "443 views",
  "chrome": "member",
  "assetRoot": "./renders",
  "markdown": "./article.md"
}
```

Markdown image paths resolve relative to the markdown file. This means a project
can reference local project assets, canonical graphics, or rendered composition
outputs:

```markdown
![Audio file drives speaker cone](../../artifacts/canonical/audio-file-to-speaker-scene.png)
```

`assets/` is for article-specific source/reference material. `renders/` is for
article-specific exported figures and GIFs that are intended to be embedded in
the markdown once a composition graduates from the graphics lab.

Only links embedded inside article markdown are clickable. Forum chrome is inert
on purpose, so preview clicks do not accidentally navigate away.

Example with an explicit asset:

```text
preview_lab/hise-published-topic.html?asset=../artifacts/canonical/audio-file-to-speaker-scene.png
```

Use `clean=1` to open with the lab panel collapsed:

```text
preview_lab/hise-published-topic.html?clean=1
```

Useful query parameters:

```text
project=../articles/hise-dsp-buffer/article.json
asset=../artifacts/canonical/audio-file-to-speaker-scene.png
chrome=member|guest
tools=1
scale=1
specimen=1
guides=1
```

This is an emulator, not a scraped copy of the forum. Keep it tuned from live
measurements when the HISE theme changes.
