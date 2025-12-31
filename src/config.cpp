#include "daemonmake/config.hpp"

#include "daemonmake/project.hpp"

#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>

namespace daemonmake {

    using json = nlohmann::json;
    namespace fs = std::filesystem;

    Config make_default_config(const std::filesystem::path& project_root) {
        // TODO: when will canonical throw?
        auto root { fs::canonical(project_root) };
        return Config {
            root,
            root / "build",
            "g++",
            "c++20",
            std::string{default_source_folder_name},
            std::string{default_include_folder_name},
            std::string{default_apps_folder_name}
        };
    }

    void to_json(json& j, const Config& c) {
        j = json{
            {"project_root", c.project_root.string()},
            {"build_directory", c.build_directory.string()},
            {"compiler", c.compiler},
            {"cxx_standard", c.cxx_standard},
            {"source_folder_name", c.source_folder_name},
            {"include_folder_name", c.include_folder_name},
            {"apps_folder_name", c.apps_folder_name}
        };
    }

    void from_json(const json& j, Config& c) {
        c.project_root = j.at("project_root").get<std::string>();
        c.build_directory = j.at("build_directory").get<std::string>();
        c.compiler = j.at("compiler").get<std::string>();
        c.cxx_standard = j.at("cxx_standard").get<std::string>();
        c.source_folder_name = j.at("source_folder_name").get<std::string>();
        c.include_folder_name = j.at("include_folder_name").get<std::string>();
        c.apps_folder_name = j.at("apps_folder_name").get<std::string>();
    }

    void save_json(const std::filesystem::path& p, const json& j) {
        fs::create_directories(p.parent_path());
        std::ofstream f { p };
        if (!f)
            throw std::runtime_error("Failed to open config file for writing: " + p.string());

        f << std::setw(4) << j << std::endl;
    }

    json load_json(const std::filesystem::path& p) {
        std::ifstream f { p };
        if (!f)
            throw std::runtime_error("Failed to open config file for reading: " + p.string());
        json j;
        f >> j;
        return j;
    }

    Config load_config(const std::filesystem::path& project_root) {
        json cfg_json(load_json(project_root / config_default_location));
        return cfg_json.get<Config>();
    }

    void save_config(const Config& cfg) {
        json j(cfg);
        save_json(cfg.project_root / config_default_location, j);
    }

}  // namespace daemonmake

