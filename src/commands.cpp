#include "daemonmake/commands.hpp"

#include "daemonmake/config.hpp"
#include "daemonmake/project.hpp"
#include "daemonmake/cmake_builder.hpp"
#include "daemonmake/daemon.hpp"

#include <filesystem>
#include <iostream>

namespace daemonmake {

namespace fs = std::filesystem;

namespace {

fs::path resolve_root(const std::string& root_arg) {
    fs::path root { root_arg.empty() ? fs::current_path() : fs::path{root_arg} };
    return fs::canonical(root);
}

void print_project_summary(const Config& cfg, const ProjectLayout& pl) {
    std::cout << "Project root: " << cfg.project_root << "\n";
    std::cout << "Build directory: " << cfg.build_directory << "\n";
    std::cout << "Compiler: " << cfg.compiler << " (" << cfg.cxx_standard << ")\n\n";

    std::cout << "Discovered " << pl.targets.size() << " targets:\n";
    for (const auto& t : pl.targets) {
        std::cout << "  - ";
        if (t.type == TargetType::Library) {
            std::cout << "lib ";
        } else {
            std::cout << "exe ";
        }
        std::cout << t.name << " ("
                  << t.source_files.size() << " sources";

        if (!t.dependencies.empty()) {
            std::cout << ", deps: ";
            for (std::size_t i = 0; i < t.dependencies.size(); ++i) {
                std::cout << t.dependencies[i];
                if (i + 1 < t.dependencies.size()) std::cout << ", ";
            }
        }
        std::cout << ")\n";
    }

    std::cout << std::endl;
}

}  // namespace


int run_init(const std::string& root_arg) {
    try {
        fs::path resolved_root { resolve_root(root_arg) };

        Config cfg { make_default_config(resolved_root) };
        save_config(cfg);

        ProjectLayout pl { make_project_layout(cfg.project_root) };
        discover_targets(cfg, pl);
        infer_target_dependencies(pl);

        print_project_summary(cfg, pl);
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "daemonmake init failed: " << ex.what() << '\n';
        return 1;
    }
}

int run_status(const std::string& root_arg) {
    try {
        fs::path resolved_root { resolve_root(root_arg) };

        Config cfg { load_config(resolved_root) };
        
        ProjectLayout pl { make_project_layout(cfg.project_root) };
        discover_targets(cfg, pl);
        infer_target_dependencies(pl);

        print_project_summary(cfg, pl);
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "daemonmake status failed: " << ex.what() << '\n';
        return 1;
    }
}

// One-time build
int run_build(const std::string& root_arg) {
    try {
        fs::path resolved_root { resolve_root(root_arg) };
        Config cfg { load_config(resolved_root) };

        ProjectLayout pl { make_project_layout(cfg.project_root) };
        discover_targets(cfg, pl);
        infer_target_dependencies(pl);

        return cmake_build(cfg, pl);
    } catch (const std::exception& ex) {
        std::cerr << "daemonmake build failed: " << ex.what() << '\n';
        return 1;
    }
}

int run_daemon(const std::string& root_arg) {
    // Constantly running?
    try {
        fs::path resolved_root { resolve_root(root_arg) };
        Config cfg { load_config(resolved_root) };

        Daemon dmon { cfg };
        return dmon.run();
    } catch (const std::exception& ex) {
        std::cerr << "daemonmake daemon failed: " << ex.what() << '\n';
        return 1;
    }
}


}  // namespace daemonmake