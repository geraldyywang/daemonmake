#include "daemonmake/daemon.hpp"

#include "daemonmake/cmake_builder.hpp"

#include <iostream>
#include <thread>
#include <chrono>

namespace daemonmake {

namespace fs = std::filesystem;

Daemon::Daemon(const Config& cfg) : cfg_ {cfg}, pl_ { make_project_layout(cfg.project_root) } {
    discover_targets(cfg_, pl_);
    infer_target_dependencies(pl_);
}

int Daemon::run() {
    snapshot_timestamps();
    std::cout << "[daemonmake] daemon running. Press Ctrl+C to stop.\n";

    int last_rc {};

    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        // TODO: need to check for new folders/libs added or deleted
        auto changed { get_changed_files() };
        if (!changed.empty()) {
            std::cout << "[daemonmake] Detected " << changed.size() << " changed file(s). Rebuilding...\n";
            last_rc = rebuild_all();
        }
    }

    return last_rc;
}

void Daemon::snapshot_timestamps() {
    last_timestamps_.clear();

    auto scan {[&](const fs::path& dir){
        if (!fs::exists(dir)) return;
        for (auto& entry : fs::recursive_directory_iterator(dir)) {
            if (!fs::is_regular_file(entry.path())) continue;
            last_timestamps_[entry.path()] = fs::last_write_time(entry.path());
        }
    }};

    scan(cfg_.project_root / cfg_.source_folder_name);
    scan(cfg_.project_root / cfg_.include_folder_name);
    scan(cfg_.project_root / cfg_.apps_folder_name);
}

std::vector<fs::path> Daemon::get_changed_files() {
    std::vector<fs::path> changed;
    std::map<fs::path, fs::file_time_type> now;

    auto scan_for_changes {[&](const fs::path& dir){
        if (!fs::exists(dir)) return;
        for (auto& entry : fs::recursive_directory_iterator(dir)) {
            if (!fs::is_regular_file(entry.path())) continue;
            now[entry.path()] = fs::last_write_time(entry.path());
        }
    }};

    scan_for_changes(cfg_.project_root / cfg_.source_folder_name);
    scan_for_changes(cfg_.project_root / cfg_.include_folder_name);
    scan_for_changes(cfg_.project_root / cfg_.apps_folder_name);

    for (const auto& [path, ts] : now) {
        if (last_timestamps_.count(path) == 0 || last_timestamps_[path] != ts) {
            changed.push_back(path);
        }
    }

    last_timestamps_ = std::move(now);
    return changed;
}

int Daemon::rebuild_all() {
    discover_targets(cfg_, pl_);
    infer_target_dependencies(pl_);
    return cmake_build(cfg_, pl_);
}

int Daemon::rebuild_changed() {
    // TODO: implement
    return 0;
}

}  // namespace daemonmake
