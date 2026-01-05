#include "daemonmake/project.hpp"

#include <fstream>
#include <iostream>
#include <unordered_set>

namespace daemonmake {

namespace fs = std::filesystem;

namespace {

std::vector<std::string> parse_includes(const fs::path& file_path) {
  std::ifstream file{file_path};
  if (!file.is_open()) return {};

  std::vector<std::string> deps;

  // TODO: Optimize in case cpp files are too long
  std::string line;
  while (std::getline(file, line)) {
    const auto is_include{line.find("#include \"") != std::string::npos};
    if (!is_include) continue;

    const auto first_quote_pos{line.find('"')};
    if (first_quote_pos == std::string::npos) continue;

    const auto second_quote_pos{line.find('"', first_quote_pos + 1)};
    if (second_quote_pos == std::string::npos) continue;

    std::string header{line.substr(first_quote_pos + 1,
                                   second_quote_pos - first_quote_pos - 1)};

    deps.push_back(std::move(header));
  }

  return deps;
}

}  // namespace

ProjectLayout make_project_layout(const std::filesystem::path& project_root) {
  return {project_root.filename().string(), fs::canonical(project_root), {}};
}

void discover_targets(const Config& cfg, ProjectLayout& pl) {
  pl.targets.clear();
  
  // If there are files that are not in a subfolder, group them into unnamed
  // target as a lib
  Target files_not_grouped{
      std::string{default_lib_name}, TargetType::Library, {}, {}, {}};

  const auto src_path{pl.project_root / cfg.source_folder_name};
  if (fs::exists(src_path)) {
    for (const auto& entry : fs::directory_iterator(src_path)) {
      if (fs::is_directory(entry)) {
        pl.targets.emplace_back(entry.path().filename().string(),
                                TargetType::Library, std::vector<std::string>{},
                                std::vector<std::string>{},
                                std::vector<std::string>{});
      } else if (fs::is_regular_file(entry) &&
                 entry.path().extension() == ".cpp") {
        files_not_grouped.source_files.push_back(
            fs::relative(entry.path(), pl.project_root).string());
      }
    }
    for (auto& target : pl.targets) {
      const auto lib_src{src_path / target.name};
      if (!fs::exists(lib_src)) continue;

      for (const auto& entry : fs::recursive_directory_iterator(lib_src)) {
        if (entry.path().extension() != ".cpp") continue;
        target.source_files.push_back(
            fs::relative(entry.path(), pl.project_root).string());
      }
    }
  }

  const auto include_path{pl.project_root / cfg.include_folder_name /
                          pl.project_name};
  if (fs::exists(include_path)) {
    for (const auto& entry : fs::directory_iterator(include_path)) {
      if (fs::is_directory(entry)) continue;
      if (!fs::is_regular_file(entry)) continue;

      const auto ext{entry.path().extension()};
      if (ext == ".hpp" || ext == ".h")
        files_not_grouped.header_files.push_back(
            fs::relative(entry.path(), pl.project_root).string());
    }

    for (auto& target : pl.targets) {
      const auto lib_inc{include_path / target.name};
      if (!fs::exists(lib_inc)) continue;

      for (const auto& entry : fs::recursive_directory_iterator(lib_inc)) {
        if (!fs::is_regular_file(entry)) continue;
        const auto ext{entry.path().extension()};
        if (ext != ".hpp" && ext != ".h") continue;

        target.header_files.push_back(
            fs::relative(entry.path(), pl.project_root).string());
      }
    }
  }

  if (!files_not_grouped.source_files.empty() ||
      !files_not_grouped.header_files.empty())
    pl.targets.push_back(std::move(files_not_grouped));

  const auto apps_path{pl.project_root / cfg.apps_folder_name};
  if (fs::exists(apps_path)) {
    for (const auto& entry : fs::directory_iterator(apps_path)) {
      if (!fs::is_regular_file(entry) || entry.path().extension() != ".cpp")
        continue;

      pl.targets.emplace_back(
          entry.path().stem().string(), TargetType::Executable,
          std::vector<std::string>{
              fs::relative(entry.path(), pl.project_root).string()},
          std::vector<std::string>{}, std::vector<std::string>{});
    }
  }
}

void infer_target_dependencies(ProjectLayout& pl) {
  for (auto& target : pl.targets) {
    std::unordered_set<std::string> unique_deps;

    auto fill_unique_deps{[&](const std::vector<std::string>& files) {
      for (const auto& rel_file_path : files) {
        const fs::path file_path{pl.project_root / rel_file_path};
        const auto dependencies{parse_includes(file_path)};
        for (const auto& header : dependencies) {
          // Assuming header is formatted <project>/<lib>/<...>.hpp
          const auto first_slash_pos{header.find('/')};
          if (first_slash_pos == std::string::npos) continue;

          const auto second_slash_pos{header.find('/', first_slash_pos + 1)};
          // If second slash is missing, assume it's default lib
          std::string lib_name{};
          if (second_slash_pos != std::string::npos) {
            lib_name = header.substr(first_slash_pos + 1,
                                     second_slash_pos - first_slash_pos - 1);
          } else {
            lib_name = default_lib_name;
          }

          unique_deps.insert(lib_name);
        }
      }
    }};

    fill_unique_deps(target.source_files);
    fill_unique_deps(target.header_files);

    unique_deps.erase(target.name);
    target.dependencies.assign(unique_deps.begin(), unique_deps.end());
  }
}

TargetGraph::TargetGraph(const ProjectLayout& pl) {
  TargetId id_counter{};

  for (const auto& target : pl.targets) {
    target_name_to_id[target.name] = id_counter++;
  }

  dependencies = std::vector<std::vector<TargetId>>(id_counter);
  reverse_dependencies = std::vector<std::vector<TargetId>>(id_counter);

  for (const auto& target : pl.targets) {
    const auto& target_id {target_name_to_id[target.name]};
    for (const auto& dep_name : target.dependencies) {
        const auto& dep_id {target_name_to_id[dep_name]};
        dependencies[target_id].push_back(dep_id);
        reverse_dependencies[dep_id].push_back(target_id);
    }
  }
}

}  // namespace daemonmake