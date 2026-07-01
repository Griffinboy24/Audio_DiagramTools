# Article Projects

Each article gets one folder:

```text
articles/<project-id>/
  article.json
  article.md
  assets/
  renders/
```

Use `article.md` for the post body. Use `renders/` for accepted article figures
and GIFs. Use `assets/` for source/reference material that belongs only to that
article.

The root preview dashboard reads `articles/index.json`. When a new article is
created, add it to that index so it appears on `preview.html`.

Create a new article scaffold from the repository root:

```powershell
.\scripts\NewArticleProject.ps1 -Slug hise-my-post -Title "[Blog] My Post"
```
