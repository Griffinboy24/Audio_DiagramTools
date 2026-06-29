#include "audio_diagram_tools/article_project.h"

#include <fstream>
#include <stdexcept>

namespace adt::article_project {
namespace {

void requireSlug(const ArticleProjectSpec& spec) {
  if (spec.slug.empty())
    throw std::runtime_error("Article project slug cannot be empty.");

  if (spec.slug.find_first_of("\\/:*?\"<>|") != std::string::npos)
    throw std::runtime_error("Article project slug contains a path separator or invalid filename character.");
}

void writeTextFile(const std::filesystem::path& path,
                   const std::string& text,
                   bool force) {
  if (!force && std::filesystem::exists(path))
    throw std::runtime_error("Refusing to overwrite existing file: " + path.string());

  std::ofstream stream(path, std::ios::binary);
  if (!stream)
    throw std::runtime_error("Could not write file: " + path.string());

  stream << text;
}

std::string jsonEscape(const std::string& text) {
  std::string result;
  result.reserve(text.size());
  for (char c : text) {
    if (c == '\\' || c == '"')
      result.push_back('\\');
    result.push_back(c);
  }
  return result;
}

std::string metadataText(const ArticleProjectSpec& spec) {
  const std::string title = spec.topic_title.empty() ? spec.slug : spec.topic_title;
  return "{\n"
         "  \"schema\": \"audio-diagram-tools.article-project.v1\",\n"
         "  \"targetProfile\": \"" + jsonEscape(spec.target_profile) + "\",\n"
         "  \"topicTitle\": \"" + jsonEscape(title) + "\",\n"
         "  \"author\": \"" + jsonEscape(spec.author) + "\",\n"
         "  \"age\": \"draft\",\n"
         "  \"category\": \"Blog Entries\",\n"
         "  \"posts\": \"1 post\",\n"
         "  \"posters\": \"1 poster\",\n"
         "  \"views\": \"draft\",\n"
         "  \"chrome\": \"member\",\n"
         "  \"assetRoot\": \"./renders\",\n"
         "  \"markdown\": \"./article.md\"\n"
         "}\n";
}

std::string markdownText(const ArticleProjectSpec& spec) {
  const std::string title = spec.topic_title.empty() ? spec.slug : spec.topic_title;
  return "## " + title + "\n\n"
         "Draft copy starts here.\n\n"
         "![diagram](./renders/diagram.gif)\n";
}

} // namespace

ArticleProjectPaths pathsFor(const ArticleProjectSpec& spec) {
  requireSlug(spec);

  ArticleProjectPaths paths;
  paths.root = spec.articles_root / spec.slug;
  paths.markdown = paths.root / "article.md";
  paths.metadata = paths.root / "article.json";
  paths.assets = paths.root / "assets";
  paths.renders = paths.root / "renders";
  return paths;
}

std::filesystem::path renderPath(const ArticleProjectPaths& paths,
                                 const std::string& asset_name,
                                 const std::string& extension) {
  if (asset_name.empty())
    throw std::runtime_error("Render asset name cannot be empty.");

  std::filesystem::path filename(asset_name);
  if (filename.has_parent_path())
    throw std::runtime_error("Render asset name must be a filename, not a path: " + asset_name);

  std::string normalized_extension = extension;
  if (!normalized_extension.empty() && normalized_extension.front() != '.')
    normalized_extension.insert(normalized_extension.begin(), '.');

  filename.replace_extension(normalized_extension);
  return paths.renders / filename;
}

ArticleProjectPaths scaffold(const ArticleProjectSpec& spec) {
  const ArticleProjectPaths paths = pathsFor(spec);

  if (!spec.force && std::filesystem::exists(paths.root))
    throw std::runtime_error("Article project already exists: " + paths.root.string());

  std::filesystem::create_directories(paths.assets);
  std::filesystem::create_directories(paths.renders);

  writeTextFile(paths.markdown, markdownText(spec), spec.force);
  writeTextFile(paths.metadata, metadataText(spec), spec.force);
  writeTextFile(paths.assets / "README.md",
                "# Project Assets\n\n"
                "Put article-specific source assets here. Reusable canonical graphics belong in code.\n",
                spec.force);
  writeTextFile(paths.renders / "README.md",
                "# Project Renders\n\n"
                "Put deliberate final PNG/GIF exports for this article here.\n",
                spec.force);

  return paths;
}

} // namespace adt::article_project
