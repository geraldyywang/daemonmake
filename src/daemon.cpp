#include "daemonmake/daemon.hpp"

#include <chrono>
#include <iostream>
#include <stop_token>
#include <thread>

#include "daemonmake/cmake_builder.hpp"
#include "daemonmake/file_watcher.hpp"

namespace daemonmake {

namespace fs = std::filesystem;

Daemon::Daemon(const Config& cfg)
    : cfg_{cfg},
      pl_{make_project_layout(cfg.project_root)},
      build_queue_{daemon_build_queue_size} {
  update_pl();
}

Daemon::~Daemon() { stop(); }

int Daemon::run() {
  const auto watcher_loop{[this](const std::stop_token& token) {
    FileWatcher watcher{{cfg_.project_root / cfg_.include_folder_name,
                         cfg_.project_root / cfg_.source_folder_name,
                         cfg_.project_root / cfg_.apps_folder_name}};
    while (!token.stop_requested()) {
      auto events{watcher.wait_for_events()};
      for (const auto& e : events) {
        build_queue_.push_event(e);
      }
    }
  }};

  const auto builder_loop{[this](const std::stop_token& token) {
    while (!token.stop_requested()) {
      auto task{build_queue_.pop_all_events(token)};
      if (task.events.empty() && !task.full_rebuild) continue;
      if (task.full_rebuild) {
        std::cout << "[daemonmake] Executing full rebuild...\n";
        rebuild_all();
      } else {
        if (task.requires_discovery()) {
          update_pl();
        }
        std::cout << "[daemonmake] Detected " << task.events.size()
                  << " changed file(s). Rebuilding...\n";
        rebuild_changed(task);
      }
    }
  }};

  builder_thread_ = std::jthread{builder_loop};
  watcher_thread_ = std::jthread{watcher_loop};

  return 0;
}

void Daemon::stop() {
  build_queue_.shutdown();
  if (watcher_thread_.joinable()) watcher_thread_.request_stop();
  if (builder_thread_.joinable()) builder_thread_.request_stop();
}

void Daemon::update_pl() {
  std::scoped_lock<std::mutex> lock{mtx_};
  discover_targets(cfg_, pl_);
  infer_target_dependencies(pl_);
  graph_ = TargetGraph{pl_};
}

int Daemon::rebuild_all() {
  update_pl();
  return cmake_build(cfg_, pl_, true);
}

int Daemon::rebuild_changed(BuildQueue::Task& task) {
  // TODO: Implement target builds for v2 (engage graph_ as well)
  // for (const auto& [path, type] : task.events) {
  // }
  return cmake_build(cfg_, pl_, task.requires_discovery());
}

}  // namespace daemonmake
