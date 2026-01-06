#ifndef DAEMONMAKE__DAEMONMAKE_CONFIG
#define DAEMONMAKE__DAEMONMAKE_CONFIG

#include <filesystem>
#include <string>

namespace daemonmake {

inline constexpr std::string_view default_source_folder_name{"src"};
inline constexpr std::string_view default_apps_folder_name{"apps"};
inline constexpr std::string_view default_include_folder_name{"include"};
inline constexpr std::string_view config_default_location{
    ".daemonmake/config.json"};

/**
 * Project configuration state.
 *
 * This struct represents the persistent settings for a daemonmake project.
 * All paths are stored as absolute canonical paths to ensure consistency
 * across different working directories.
 */
struct Config {
  std::filesystem::path project_root;
  std::filesystem::path build_directory;

  std::string compiler;
  std::string cxx_standard;

  std::string source_folder_name;
  std::string include_folder_name;
  std::string apps_folder_name;
};

/**
 * Generates a default configuration object for a project root.
 *
 * Canonicalizes the provided path and sets default values for the compiler
 * (g++), standard (c++20), and folder structure (src, include, apps).
 *
 * @param project_root The base directory of the project.
 * @return A Config object with default settings.
 */
Config make_default_config(const std::filesystem::path& project_root);

/**
 * Loads and parses the configuration from the project's JSON config file.
 *
 * Looks for the config file at <project_root>/.daemonmake/config.json.
 *
 * @param project_root The base directory of the project.
 * @return The parsed Config object.
 * @throws std::runtime_error If the file is missing or contains invalid JSON.
 */
Config load_config(const std::filesystem::path& project_root);

/**
 * Persists the configuration to the project's JSON config file.
 *
 * Automatically creates the .daemonmake directory if it does not exist.
 *
 * @param cfg The configuration object to save.
 * @throws std::runtime_error If the file cannot be written.
 */
void save_config(const Config& cfg);

}  // namespace daemonmake

#endif