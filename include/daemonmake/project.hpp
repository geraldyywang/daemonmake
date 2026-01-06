#ifndef DAEMONMAKE__DAEMONMAKE_PROJECT
#define DAEMONMAKE__DAEMONMAKE_PROJECT

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "daemonmake/config.hpp"

namespace daemonmake {

inline constexpr std::string_view default_lib_name{"UnnamedLib"};

/**
 * Indicates the build artifact type for a target.
 */
enum class TargetType { Library, Executable };

/**
 * Represents a build artifact discovered in the project.
 *
 * Each target groups a set of source and header files. Its dependencies are
 * discovered through static analysis of include directives.
 */
struct Target {
  std::string name;
  TargetType type;
  std::vector<std::string> source_files;
  std::vector<std::string> header_files;
  std::vector<std::string> dependencies;
};

/**
 * A snapshot of the physical and logical structure of the project.
 */
struct ProjectLayout {
  std::string project_name;
  std::filesystem::path project_root;
  std::vector<Target> targets;
};

using TargetId = uint32_t;

/**
 * A directed graph representing the build dependency hierarchy.
 *
 * Provides efficient lookups for both direct dependencies (what I need)
 * and reverse dependencies (who needs me).
 */
struct TargetGraph {
  std::unordered_map<std::string, TargetId> target_name_to_id;
  std::vector<std::vector<TargetId>> dependencies;
  std::vector<std::vector<TargetId>> reverse_dependencies;

  TargetGraph() = default;
  /**
   * Constructs the graph from a fully discovered ProjectLayout.
   * 
   * @param pl The layout used to populate nodes and edges.
   */
  TargetGraph(const ProjectLayout& pl);
};

/**
 * Creates a base ProjectLayout for a given root directory.
 *
 * @param project_root The absolute path to the project.
 * @return An initialized layout with name and root path set.
 */
ProjectLayout make_project_layout(const std::filesystem::path& project_root);

/**
 * Scans the filesystem to identify libraries and executables.
 *
 * Logic:
 * - Subdirectories in 'src/' become Library targets.
 * - Files in 'apps/' become individual Executable targets.
 * - Headers are associated based on <project_name>/<target_name> structure.
 *
 * @param cfg The project configuration.
 * @param pl The layout to populate with discovered targets.
 */
void discover_targets(const Config& cfg, ProjectLayout& pl);

/**
 * Analyzes file contents to find inter-target dependencies.
 *
 * Parses #include "project/target/..." strings to map relationships.
 * @param pl The layout to update with dependency metadata.
 */
void infer_target_dependencies(ProjectLayout& pl);

}  // namespace daemonmake

#endif