#pragma once

#include <filesystem>
#include <string>

namespace adt::article_project {

struct ArticleProjectSpec {
  std::filesystem::path articles_root = "articles";
  std::string slug;
  std::string topic_title;
  std::string author = "griffinboy";
  std::string target_profile = "hise-published-topic";
  bool force = false;
};

struct ArticleProjectPaths {
  std::filesystem::path root;
  std::filesystem::path markdown;
  std::filesystem::path metadata;
  std::filesystem::path assets;
  std::filesystem::path renders;
};

ArticleProjectPaths pathsFor(const ArticleProjectSpec& spec);
std::filesystem::path renderPath(const ArticleProjectPaths& paths,
                                 const std::string& asset_name,
                                 const std::string& extension);
ArticleProjectPaths scaffold(const ArticleProjectSpec& spec);

} // namespace adt::article_project
