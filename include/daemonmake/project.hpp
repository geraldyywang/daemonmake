#ifndef DAEMONMAKE__DAEMONMAKE_PROJECT
#define DAEMONMAKE__DAEMONMAKE_PROJECT

#include <vector>
#include <string>
#include <string_view>
#include <filesystem>

namespace daemonmake {

inline constexpr std::string_view default_lib_name { "UnnamedLib" };

enum class TargetType { Library, Executable };

/**
 * Represents a library or executable target discovered in the project.
 * Contains its sources, headers, and inferred target dependencies.
 */
struct Target {
    std::string name;
    TargetType type;
    std::vector<std::string> source_files;
    std::vector<std::string> header_files;
    std::vector<std::string> dependencies;
};

/**
 * High-level description of a project’s structure, including its name,
 * root path, and all discovered build targets.
 */
struct ProjectLayout {
    std::string project_name;
    std::filesystem::path project_root;
    std::vector<Target> targets;
};

/**
 * Initializes a ProjectLayout for the given root. Does not scan the
 * filesystem; call discover_targets() and infer_target_dependencies()
 * afterwards to populate the layout.
 */
ProjectLayout make_project_layout(const std::filesystem::path& project_root);

/**
 * Scans the project tree and populates pl.targets.
 *
 * Libraries:
 *   - One target per subdirectory in <root>/src.
 *   - A fallback "UnnamedLib" target for top-level sources/headers.
 *
 * Executables:
 *   - One target per .cpp file in <root>/apps.
 *
 * Headers:
 *   - Collected from <root>/include/<project>/<target>.
 *
 * Assumes pl.project_root and pl.project_name are already initialized.
 */
void discover_targets(const Config& cfg, ProjectLayout& pl);

/**
 * Infers inter-target dependencies by parsing #include directives in each
 * target's sources and headers. Matches includes of the form:
 *
 *    "<project>/<lib>/...>"
 *
 * and records <lib> as a dependency. Includes without a second path
 * component map to the default library.
 */
void infer_target_dependencies(ProjectLayout& pl);
    
}  // namespace daemonmake

#endif