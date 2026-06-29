#include "audio_diagram_tools/article_project.h"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>

namespace {

void printUsage() {
  std::cout
      << "Usage:\n"
      << "  adt_create_article_project --slug hise-my-post --title \"[Blog] My Post\"\n\n"
      << "Options:\n"
      << "  --slug <id>          Article folder name under articles/.\n"
      << "  --title <text>       Topic/article title.\n"
      << "  --author <name>      Author name. Defaults to griffinboy.\n"
      << "  --articles-root <p>  Root articles folder. Defaults to articles.\n"
      << "  --force              Overwrite scaffold files if they already exist.\n";
}

adt::article_project::ArticleProjectSpec parseArgs(int argc, char** argv) {
  adt::article_project::ArticleProjectSpec spec;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    auto requireValue = [&](std::string_view option) -> std::string {
      if (i + 1 >= argc)
        throw std::runtime_error("Missing value for " + std::string(option));
      return argv[++i];
    };

    if (arg == "--help" || arg == "-h") {
      printUsage();
      std::exit(0);
    }
    else if (arg == "--slug") {
      spec.slug = requireValue(arg);
    }
    else if (arg == "--title") {
      spec.topic_title = requireValue(arg);
    }
    else if (arg == "--author") {
      spec.author = requireValue(arg);
    }
    else if (arg == "--articles-root") {
      spec.articles_root = requireValue(arg);
    }
    else if (arg == "--force") {
      spec.force = true;
    }
    else {
      throw std::runtime_error("Unknown option: " + arg);
    }
  }

  return spec;
}

} // namespace

int main(int argc, char** argv) {
  try {
    const adt::article_project::ArticleProjectSpec spec = parseArgs(argc, argv);
    const adt::article_project::ArticleProjectPaths paths =
        adt::article_project::scaffold(spec);

    std::cout << "Created article project:\n"
              << "  root:     " << paths.root.string() << "\n"
              << "  markdown: " << paths.markdown.string() << "\n"
              << "  metadata: " << paths.metadata.string() << "\n";
    return 0;
  }
  catch (const std::exception& e) {
    std::cerr << "adt_create_article_project: " << e.what() << "\n\n";
    printUsage();
    return 1;
  }
}
