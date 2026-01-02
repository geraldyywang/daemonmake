#ifndef DAEMONMAKE__DAEMONMAKE_CONFIG
#define DAEMONMAKE__DAEMONMAKE_CONFIG

#include <string>
#include <filesystem>

namespace daemonmake {

inline constexpr std::string_view config_default_location { ".daemonmake/config.json" };

/**
 * Project configuration loaded from or written to `.daemonmake/config.json`.
 *
 * Paths are stored as absolute canonical paths. Folder names determine where
 * targets are discovered (e.g. src/, include/, apps/).
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
 * Creates a default configuration for the given project root.
 *
 * The root is canonicalized, and default paths/values are populated:
 *   - build directory: <root>/build
 *   - compiler: "g++"
 *   - cxx_standard: "c++20"
 *   - folder names: src/, include/, apps/
 */
Config make_default_config(const std::filesystem::path& project_root);

/**
 * Loads configuration from `<project_root>/.daemonmake/config.json`.
 *
 * Throws std::runtime_error if the file cannot be opened or parsed. The
 * returned Config contains absolute canonical paths and user-customized
 * directory names.
 */
Config load_config(const std::filesystem::path& project_root);

/**
 * Serializes the configuration to `<cfg.project_root>/.daemonmake/config.json`.
 *
 * Creates parent directories if needed and overwrites the file. Throws
 * std::runtime_error on I/O failure.
 */
void save_config(const Config& cfg);


}  // namespace daemonmake

#endif