#include "daemonmake/cmake_builder.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

namespace daemonmake {

namespace fs = std::filesystem;

namespace {

// Helper: extract the numeric C++ standard (e.g. "20") from a string like "c++20".
std::string extract_cxx_standard_number(const std::string& standard) {
    std::string digits;
    for (char ch : standard) {
        if (ch >= '0' && ch <= '9') {
            digits.push_back(ch);
        } else if (!digits.empty()) {
            break;
        }
    }
    if (digits.empty()) {
        return "20";
    }
    return digits;
}

// Helper: choose a CMake project name from the project root path.
std::string derive_project_name(const fs::path& root) {
    if (root.has_filename()) {
        auto name = root.filename().string();
        if (!name.empty()) return name;
    }
    return "daemonmake_project";
}

}  // namespace


int cmake_build(const Config& cfg) {
    fs::create_directories(cfg.build_directory);

    if (!fs::exists(cfg.project_root / "CMakeLists.txt")) {
        ProjectLayout pl { make_project_layout(cfg.project_root) };
        discover_targets(pl);
        infer_target_dependencies(pl);

        write_cmakelists(cfg, pl);
    }

    const std::string configure_cmd { 
        "cmake -S " + cfg.project_root.string() + " -B " + cfg.build_directory.string() 
    };

    std::cout << "[daemonmake] " << configure_cmd << '\n';
    int rc { std::system(configure_cmd.c_str()) };
    if (rc != 0) {
        std::cerr << "daemonmake build: CMake configuration failed (rc=" << rc << ")\n";
    }

    const std::string build_cmd {
        "cmake --build " + cfg.build_directory.string()
    };

    std::cout << "[daemonmake] " << build_cmd << '\n';
    rc = std::system(build_cmd.c_str());
    if (rc != 0) {
        std::cerr << "daemonmake build: CMake build failed (rc=" << rc << ")\n";
    }

    return rc;
}


void write_cmakelists(const Config& cfg, const ProjectLayout& pl, bool overwrite) {
    const fs::path cmake_path = cfg.project_root / "CMakeLists.txt";

    if (fs::exists(cmake_path) && !overwrite) {
        throw std::runtime_error(
            "CMakeLists.txt already exists at " + cmake_path.string() +
            " (refusing to overwrite; pass overwrite=true if you really want to).");
    }

    std::ostringstream oss;

    const std::string project_name = derive_project_name(cfg.project_root);
    const std::string cxx_std_num  = extract_cxx_standard_number(cfg.cxx_standard);

    oss << "cmake_minimum_required(VERSION 3.20)\n";
    oss << "project(" << project_name << " LANGUAGES CXX)\n\n";

    oss << "set(CMAKE_CXX_STANDARD " << cxx_std_num << ")\n";
    oss << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n";
    oss << "set(CMAKE_CXX_EXTENSIONS OFF)\n\n";

    oss << "# Compiler configured by daemonmake\n";
    oss << "if (NOT CMAKE_CXX_COMPILER)\n";
    oss << "    set(CMAKE_CXX_COMPILER \"" << cfg.compiler << "\")\n";
    oss << "endif()\n\n";

    oss << "# Assume public headers live under include/\n";
    oss << "set(PROJECT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)\n\n";

    oss << "# Targets discovered by daemonmake\n\n";

    for (const auto& t : pl.targets) {
        oss << "add_";
        if (t.type == TargetType::Library) {
            oss << "library(";
        } else {
            oss << "executable(";
        }

        oss << t.name;

        if (!t.source_files.empty()) {
            oss << "\n";
            for (const auto& src : t.source_files) {
                oss << "    " << src << "\n";
            }
        }

        oss << ")\n\n";

        oss << "target_include_directories(" << t.name
            << " PRIVATE ${PROJECT_INCLUDE_DIR})\n\n";
    }

    oss << "# Inferred dependencies between targets\n\n";

    for (const auto& t : pl.targets) {
        if (t.dependencies.empty()) continue;

        oss << "target_link_libraries(" << t.name << "\n";
        oss << "    PRIVATE\n";
        for (const auto& dep : t.dependencies) {
            oss << "        " << dep << "\n";
        }
        oss << ")\n\n";
    }

    fs::create_directories(cmake_path.parent_path());

    std::ofstream out{cmake_path};
    if (!out) {
        throw std::runtime_error("Failed to open " + cmake_path.string() +
                                 " for writing CMakeLists.txt");
    }

    out << oss.str();
    out.flush();
    if (!out) {
        throw std::runtime_error("Failed to write CMakeLists.txt to " +
                                 cmake_path.string());
    }
}

} // namespace daemonmake
