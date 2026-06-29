(function () {
  const DEFAULT_MARKDOWN = [
    '## Back From Hibernation: My Favourite C++ Open Source DSP References',
    '',
    'Hi HISE forum!',
    '',
    'It has been a while since I have made a proper post here. I used to be a lot more active, then work swallowed me whole for what feels like a couple of years.',
    '',
    'In that time I have mostly been writing C++ DSP, analog modelling, optimisation, and the little details that make audio code really solid.',
    '',
    '![Audio file drives speaker cone](../artifacts/canonical/audio-file-to-speaker-scene.png)',
    '',
    'Hello this is more text in-between.',
    'A bit of text, how nice! Hello this is just testing out what it looks like.',
    'When I write out a HISE Post with images. Jaa sou iu koto da, ii kanji deshou ne. Hello this is more text in-between.',
    '',
    '### ResearchGate',
    '',
    '[ResearchGate](https://www.researchgate.net/)',
    '',
    'This is one of the places I use to find research papers. I use other sites as well, but ResearchGate is my first go-to.'
  ].join('\n');

  const DEFAULT_ARTICLE = {
    topicTitle: '[Blog] My Favourite C++ Open Source DSP References',
    author: 'griffinboy',
    age: '22 days ago',
    category: 'Blog Entries',
    posts: '10 posts',
    posters: '5 posters',
    views: '443 views',
    chrome: 'member',
    markdownText: DEFAULT_MARKDOWN
  };

  const body = document.body;
  const projectInput = document.getElementById('projectPath');
  const loadProjectButton = document.getElementById('loadProject');
  const assetInput = document.getElementById('assetPath');
  const imageBackground = document.getElementById('imageBackground');
  const sessionChrome = document.getElementById('sessionChrome');
  const showGuides = document.getElementById('showGuides');
  const showScale = document.getElementById('showScale');
  const showSpecimen = document.getElementById('showSpecimen');
  const toggleLab = document.getElementById('toggleLab');
  const scaleReport = document.getElementById('scaleReport');
  const articleBody = document.getElementById('articleBody');

  const params = new URLSearchParams(window.location.search);
  let currentProjectUrl = null;
  let currentMarkdownBaseUrl = window.location.href;

  if (typeof window.markdownit !== 'function')
    throw new Error('markdown-it did not load. Check preview_lab/vendor/markdown-it-14.1.0.min.js.');

  const markdown = window.markdownit({
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

  function resolveUrl(path, base) {
    if (!path)
      return '';
    try {
      return new URL(path, base || window.location.href).toString();
    } catch (error) {
      return path;
    }
  }

  const defaultLinkOpen = markdown.renderer.rules.link_open || function (tokens, index, options, env, self) {
    return self.renderToken(tokens, index, options);
  };

  markdown.renderer.rules.link_open = function (tokens, index, options, env, self) {
    const token = tokens[index];
    const hrefIndex = token.attrIndex('href');
    if (hrefIndex >= 0) {
      const href = token.attrs[hrefIndex][1];
      token.attrs[hrefIndex][1] = resolveUrl(href, currentMarkdownBaseUrl);
    }
    token.attrSet('rel', 'nofollow ugc');
    return defaultLinkOpen(tokens, index, options, env, self);
  };

  markdown.renderer.rules.image = function (tokens, index) {
    const token = tokens[index];
    const src = token.attrGet('src') || '';
    const title = token.attrGet('title') || '';
    const alt = token.content || '';
    const resolved = resolveUrl(src, currentMarkdownBaseUrl);
    const titleAttr = title ? ` title="${escapeAttribute(title)}"` : '';

    return [
      '<figure class="hise-upload">',
      `<img class="preview-asset" src="${escapeAttribute(resolved)}" alt="${escapeAttribute(alt)}"${titleAttr}>`,
      '<figcaption class="figure-scale" aria-live="polite"></figcaption>',
      '</figure>'
    ].join('');
  };

  function setText(selector, value) {
    const node = document.querySelector(selector);
    if (node)
      node.textContent = value;
  }

  function setClass(prefix, value) {
    for (const name of [...body.classList]) {
      if (name.startsWith(prefix))
        body.classList.remove(name);
    }
    if (value)
      body.classList.add(`${prefix}${value}`);
  }

  function applyControls() {
    body.classList.toggle('show-guides', showGuides.checked);
    body.classList.toggle('show-scale', showScale.checked);
    body.classList.toggle('show-specimen', showSpecimen.checked);
    setClass('image-bg-', imageBackground.value === 'normal' ? '' : imageBackground.value);
    setClass('session-', sessionChrome.value);
    updateScaleReports();
  }

  function renderMarkdown(markdownText, markdownBaseUrl) {
    currentMarkdownBaseUrl = markdownBaseUrl || currentProjectUrl || window.location.href;
    const rendered = markdown.render(markdownText)
      .replace(/<p>\s*(<figure class="hise-upload">[\s\S]*?<\/figure>)\s*<\/p>/g, '$1');
    articleBody.innerHTML = rendered;
    wireFigureReports();
  }

  async function loadText(url) {
    const response = await fetch(url, { cache: 'no-store' });
    if (!response.ok)
      throw new Error(`Could not load ${url}: ${response.status}`);
    return response.text();
  }

  async function loadJson(url) {
    const response = await fetch(url, { cache: 'no-store' });
    if (!response.ok)
      throw new Error(`Could not load ${url}: ${response.status}`);
    return response.json();
  }

  function applyArticleConfig(article) {
    setText('.topic-title .fs-3', article.topicTitle || DEFAULT_ARTICLE.topicTitle);
    setText('.post-author-name', article.author || DEFAULT_ARTICLE.author);
    const postTime = document.querySelector('.post-time');
    if (postTime)
      postTime.innerHTML = `<i class="fa fa-fw fa-pencil-square"></i> ${escapeHtml(article.age || DEFAULT_ARTICLE.age)}`;

    const metaItems = document.querySelectorAll('.topic-meta > span');
    if (metaItems[0])
      metaItems[0].innerHTML = `<i class="fa fa-fw fa-pencil"></i> ${escapeHtml(article.category || DEFAULT_ARTICLE.category)}`;
    if (metaItems[1])
      metaItems[1].textContent = article.posts || DEFAULT_ARTICLE.posts;
    if (metaItems[2])
      metaItems[2].textContent = article.posters || DEFAULT_ARTICLE.posters;
    if (metaItems[3])
      metaItems[3].textContent = article.views || DEFAULT_ARTICLE.views;

    if (article.chrome && !params.has('chrome'))
      sessionChrome.value = article.chrome;
  }

  function articleWithAssetOverride(markdownText, assetPath) {
    if (!assetPath)
      return markdownText;

    const replacement = `![Preview asset](${assetPath})`;
    if (/!\[[^\]]*\]\([^)]+\)/m.test(markdownText))
      return markdownText.replace(/!\[[^\]]*\]\([^)]+\)/m, replacement);
    return `${markdownText}\n\n${replacement}`;
  }

  async function loadProject(path) {
    const projectUrl = resolveUrl(path, window.location.href);
    currentProjectUrl = projectUrl;
    try {
      const project = await loadJson(projectUrl);
      applyArticleConfig(project);
      const markdownUrl = project.markdown
        ? resolveUrl(project.markdown, projectUrl)
        : projectUrl;
      const markdownText = project.markdownText || await loadText(markdownUrl);
      currentProjectUrl = markdownUrl;
      renderMarkdown(articleWithAssetOverride(markdownText, assetInput.value.trim()), markdownUrl);
      scaleReport.textContent = 'Article loaded.';
    } catch (error) {
      currentProjectUrl = window.location.href;
      applyArticleConfig(DEFAULT_ARTICLE);
      renderMarkdown(articleWithAssetOverride(DEFAULT_ARTICLE.markdownText, assetInput.value.trim()), window.location.href);
      scaleReport.textContent = `Using built-in article. ${error.message}`;
    }
    applyControls();
  }

  function updateScaleReports() {
    const images = [...document.querySelectorAll('.preview-asset')];
    if (!images.length) {
      scaleReport.textContent = 'No figure assets in article.';
      return;
    }

    const reports = images.map((image, index) => {
      const caption = image.closest('figure')?.querySelector('.figure-scale');
      if (!image.complete || image.naturalWidth === 0) {
        if (caption)
          caption.textContent = 'Asset not loaded yet.';
        return `figure ${index + 1}: loading`;
      }

      const rect = image.getBoundingClientRect();
      const scale = rect.width / image.naturalWidth;
      const text = [
        `natural ${image.naturalWidth}x${image.naturalHeight}`,
        `shown ${Math.round(rect.width)}x${Math.round(rect.height)}`,
        `scale ${(scale * 100).toFixed(1)}%`
      ].join(' | ');

      if (caption)
        caption.textContent = text;
      return `figure ${index + 1}: ${text}`;
    });

    scaleReport.textContent = reports.slice(0, 3).join('\n');
  }

  function wireFigureReports() {
    for (const image of document.querySelectorAll('.preview-asset'))
      image.addEventListener('load', updateScaleReports, { once: false });
    updateScaleReports();
  }

  projectInput.value = params.get('project') || projectInput.value;
  assetInput.value = params.get('asset') || assetInput.value;
  sessionChrome.value = params.get('chrome') || sessionChrome.value;
  showSpecimen.checked = params.get('specimen') === '1';
  showGuides.checked = params.get('guides') === '1';
  showScale.checked = params.get('scale') === '1';

  if (params.get('tools') !== '1' || params.get('clean') === '1') {
    body.classList.add('lab-collapsed');
    toggleLab.textContent = 'Show';
  }

  loadProject(projectInput.value);
  applyControls();

  loadProjectButton.addEventListener('click', () => loadProject(projectInput.value));
  projectInput.addEventListener('keydown', (event) => {
    if (event.key === 'Enter')
      loadProject(projectInput.value);
  });
  assetInput.addEventListener('change', () => loadProject(projectInput.value));
  assetInput.addEventListener('keydown', (event) => {
    if (event.key === 'Enter')
      loadProject(projectInput.value);
  });
  imageBackground.addEventListener('change', applyControls);
  sessionChrome.addEventListener('change', applyControls);
  showGuides.addEventListener('change', applyControls);
  showScale.addEventListener('change', applyControls);
  showSpecimen.addEventListener('change', applyControls);
  toggleLab.addEventListener('click', () => {
    body.classList.toggle('lab-collapsed');
    toggleLab.textContent = body.classList.contains('lab-collapsed') ? 'Show' : 'Hide';
  });
  window.addEventListener('resize', updateScaleReports);
})();
