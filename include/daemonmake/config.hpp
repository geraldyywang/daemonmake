#ifndef DAEMONMAKE__DAEMONMAKE_CONFIG
#define DAEMONMAKE__DAEMONMAKE_CONFIG

#include <string>
#include <filesystem>

namespace daemonmake {

inline constexpr std::string_view config_default_location { ".daemonmake/config.json" };

struct Config {
    std::filesystem::path project_root;
    std::filesystem::path build_directory;

    std::string compiler;
    std::string cxx_standard;
    
    // TODO: add project folder paths like source?
    std::string source_folder_name;
    std::string include_folder_name;
    std::string apps_folder_name;
};

Config make_default_config(const std::filesystem::path& project_root);
Config load_config(const std::filesystem::path& project_root);
void save_config(const Config& cfg);


}  // namespace daemonmake

#endif