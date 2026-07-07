const fs = require('fs');
const path = require('path');

const root = path.resolve(__dirname, '..');
const projectPath = path.join(root, 'articles', 'hise-dsp-buffer', 'article.json');
const project = JSON.parse(fs.readFileSync(projectPath, 'utf8'));
const markdownPath = path.resolve(path.dirname(projectPath), project.markdown);
const markdownText = fs.readFileSync(markdownPath, 'utf8');
const markdownIt = require('../preview_lab/vendor/markdown-it-14.1.0.min.js');

const md = markdownIt({
  html: false,
  linkify: false,
  typographer: false,
  breaks: true
});

function escapeHtml(value) {
  return String(value)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#39;');
}

function escapeAttribute(value) {
  return escapeHtml(value).replace(/`/g, '&#96;');
}

function toRootRelative(url) {
  if (!url.startsWith('./'))
    return url;

  return path.posix.join(
    'articles/hise-dsp-buffer',
    url.slice(2).replace(/\\/g, '/')
  );
}

const defaultLinkOpen = md.renderer.rules.link_open || function (tokens, index, options, env, self) {
  return self.renderToken(tokens, index, options);
};

md.renderer.rules.link_open = function (tokens, index, options, env, self) {
  const token = tokens[index];
  const hrefIndex = token.attrIndex('href');
  if (hrefIndex >= 0)
    token.attrs[hrefIndex][1] = toRootRelative(token.attrs[hrefIndex][1]);
  token.attrSet('rel', 'nofollow ugc');
  return defaultLinkOpen(tokens, index, options, env, self);
};

md.renderer.rules.image = function (tokens, index) {
  const token = tokens[index];
  const src = toRootRelative(token.attrGet('src') || '');
  const title = token.attrGet('title') || '';
  const alt = token.content || '';
  const titleAttr = title ? ` title="${escapeAttribute(title)}"` : '';

  return [
    '<figure class="hise-upload">',
    `<img class="preview-asset" src="${escapeAttribute(src)}" alt="${escapeAttribute(alt)}"${titleAttr}>`,
    '</figure>'
  ].join('');
};

const rendered = md.render(markdownText)
  .replace(/<p>\s*(<figure class="hise-upload">[\s\S]*?<\/figure>)\s*<\/p>/g, '$1');

const html = `<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>${escapeHtml(project.topicTitle)}</title>
    <link rel="stylesheet" href="preview_lab/hise-published-topic.css?v=20260707-static-preview">
  </head>
  <body class="session-${escapeAttribute(project.chrome || 'member')}">
    <header class="hise-topbar">
      <div class="hise-topbar-inner">
        <span class="hise-brand" aria-label="HISE forum">
          <img class="hise-logo-img" src="preview_lab/assets/hise-site-logo.png" alt="">
          <span class="hise-forum">Forum</span>
        </span>
        <span class="hise-menu" aria-label="Categories"><i class="fa fa-fw fa-list"></i></span>
        <nav class="hise-member-left" aria-label="Member shortcuts">
          <span class="hise-icon-small"><i class="fa fa-clock-o"></i></span>
          <span class="hise-badge-icon"><i class="fa fa-fw fa-book"></i><b>6</b></span>
          <span class="hise-icon-small"><i class="fa fa-user"></i></span>
        </nav>
        <nav class="hise-actions" aria-label="Forum actions">
          <span class="hise-icon"><i class="fa fa-lg fa-fw fa-search"></i></span>
          <span class="hise-icon"><i class="fa fa-fw fa-lightbulb-o"></i></span>
          <span class="hise-member-action"><i class="fa fa-bell-o"></i></span>
          <span class="hise-member-action"><i class="fa fa-comment-o"></i></span>
          <span class="hise-mini-avatar" aria-hidden="true"></span>
          <span class="hise-guest-action">Register</span>
          <span class="hise-guest-action">Login</span>
        </nav>
      </div>
    </header>

    <main class="hise-page">
      <section class="topic-header">
        <h1 class="topic-title-heading">
          <div class="topic-title">
            <span class="fs-3" component="topic/title">${escapeHtml(project.topicTitle)}</span>
          </div>
        </h1>
        <div class="topic-meta">
          <span><i class="fa fa-fw fa-pencil"></i> ${escapeHtml(project.category || 'Blog Entries')}</span>
          <span>${escapeHtml(project.posts || '1 post')}</span>
          <span>${escapeHtml(project.posters || '1 poster')}</span>
          <span>${escapeHtml(project.views || 'draft')}</span>
          <span><i class="fa fa-rss-square"></i></span>
          <div class="topic-member-tools" aria-label="Topic tools">
            <span><i class="fa fa-fw fa-print"></i></span>
            <span><i class="fa fa-fw fa-bell-slash-o"></i></span>
            <span><i class="fa fa-fw fa-sort"></i></span>
            <span><i class="fa fa-fw fa-gear"></i></span>
          </div>
          <button class="reply-button"><span class="guest-label">Log in to reply</span><span class="member-label">Reply</span></button>
          <button class="reply-menu" aria-label="Reply menu"><i class="fa fa-caret-down"></i></button>
        </div>
      </section>

      <article class="post-card">
        <aside class="post-author" aria-label="Post author">
          <div class="avatar" aria-hidden="true"></div>
          <span class="post-author-name">${escapeHtml(project.author || 'griffinboy')}</span>
        </aside>

        <section class="post-content" aria-label="Post content">
          <div class="post-time"><i class="fa fa-fw fa-pencil-square"></i> ${escapeHtml(project.age || 'draft')}</div>
          <div class="markdown-body">
${rendered.trim()}
          </div>
        </section>
      </article>
    </main>
  </body>
</html>
`;

fs.writeFileSync(path.join(root, 'preview.html'), html, 'utf8');
console.log('Wrote preview.html');
