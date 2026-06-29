# Project Structure

This project has three different kinds of files. Keep them separate.

## Reusable Code

- `src/audio_diagram_tools/canonical_components.h`
  Accepted reusable Griffinboy graphics, IDs, preferred dimensions, option
  structs, and component builders such as `audioFilePlayerGraphic()`.
- `src/audio_diagram_tools/canonical_renderers.h`
  Internal renderer boundary used by the canonical registry. Callers should
  normally use `canonical_components.h`, not renderer internals.
- `src/audio_diagram_tools/canonical_graphics.cpp`
  Current Visage renderer backend for canonical graphics. It is allowed to be
  large while visual geometry is still being consolidated, but public dispatch
  and metadata belong in `canonical_components.cpp`.
- `src/audio_diagram_tools/composition.h`
  HISE-target profiles, content width helpers, alignment helpers, and vertical
  stack composition utilities.
- `examples/`
  Small command-line renderers and smoke tools.

Canonical graphics are code. PNGs and GIFs are generated previews only.

## Article Projects

Article mockups live under `articles/<project-id>/`.

Use this shape:

- `article.md`
  The article body used by the preview lab.
- `article.json`
  Project metadata and asset list.
- `assets/`
  Project-specific source assets that are not reusable framework pieces.
- `renders/`
  Optional checked-in final exports if we deliberately decide a final render
  belongs with the article project.

Use article projects for recoverability: if we return to an old HISE post, its
copy, chosen graphics, and final exports should be easy to find together.

Create a new project with:

```powershell
.\out\build\adt_create_article_project.exe --slug hise-my-post --title "[Blog] My Post"
```

or, from the repository root after building:

```powershell
.\scripts\NewArticleProject.ps1 -Slug hise-my-post -Title "[Blog] My Post"
```

Use `--articles-root <path>` to scaffold somewhere else and `--force` only when
you deliberately want to refresh scaffold files.

## Generated Scratch Output

Use `artifacts/` for previews, experiments, frame sequences, GIF tests, and
temporary visual QA output. This directory is ignored by git.

Do not treat anything in `artifacts/` as canonical. If a visual is accepted,
promote the code path or project file that creates it.

## Preview Lab

The HISE preview lab lives in `preview_lab/` and reads article projects from
`articles/`. It should emulate the published HISE topic page closely enough to
judge scale, wrapping, image placement, and article rhythm.

The lab is not a project manager. Projects are chosen by file paths and JSON,
because the intended workflow is code-driven: create graphics, write/update
article markdown, render/preview, iterate.

## Composition Conventions

For HISE article graphics, default to `composition::hiseArticleImageProfile()`.
It uses a 920 px output width and an 800 px content band. Prefer the incremental
stack builder for article composites:

```cpp
composition::StackBuilder builder(composition::hiseArticleImageProfile(),
                                  { 56.0f, 24.0f, 58.0f, false });
builder.add(canonical::audioFilePlayerGraphic(), 34.0f)
    .add(canonical::doubleArrowGraphic(), 24.0f)
    .add(canonical::speakerAnimationGraphic(), 22.0f);

composition::Scene scene = builder.build();
composition::addCenteredLabelBodyLine(
    scene, "Top:", "Sound over time (waveform).", 552.0f, 380.0f);
```

Override `x_offset`, `gap_after`, or explicit dimensions only when the article
needs that particular graphic to break the default rhythm.

Use `article_project::renderPath()` when a script needs to place final exports
inside a project `renders/` folder.
