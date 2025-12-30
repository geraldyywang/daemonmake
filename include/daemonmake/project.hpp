#ifndef DAEMONMAKE__DAEMONMAKE_PROJECT
#define DAEMONMAKE__DAEMONMAKE_PROJECT

#include <vector>
#include <string>
#include <string_view>
#include <filesystem>

namespace daemonmake {

inline constexpr std::string_view default_source_folder_name { "src" };
inline constexpr std::string_view default_apps_folder_name { "apps" };
inline constexpr std::string_view default_include_folder_name { "include" };
inline constexpr std::string_view default_lib_name { "UnnamedLib" };

enum class TargetType { Library, Executable };

struct Target {
    std::string name;
    TargetType type;
    // .cpp files
    std::vector<std::string> source_files;
    std::vector<std::string> header_files;
    // Names of other targets this target depends on (by target.name)
    std::vector<std::string> dependencies;
};

struct ProjectLayout {
    std::filesystem::path project_root;
    std::vector<Target> targets;
};

ProjectLayout make_project_layout(const std::filesystem::path& project_root);

void discover_targets(ProjectLayout& pl);

void infer_target_dependencies(ProjectLayout& pl);
    
}  // namespace daemonmake

#endif