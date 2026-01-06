#ifndef DAEMONMAKE__DAEMONMAKE_DAEMON
#define DAEMONMAKE__DAEMONMAKE_DAEMON

#include <filesystem>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include "daemonmake/build_queue.hpp"
#include "daemonmake/config.hpp"
#include "daemonmake/project.hpp"

namespace daemonmake {

inline constexpr ssize_t daemon_build_queue_size{1000};

/**
 * Orchestrates the background build service.
 *
 * The Daemon manages two persistent background threads:
 * 1. A FileWatcher thread that monitors the filesystem and produces events.
 * 2. A Builder thread that consumes events and executes build commands.
 *
 * This class is thread-safe; internal state like the ProjectLayout is protected
 * by a mutex to allow concurrent access between discovery and build phases.
 */
class Daemon {
 public:
  /**
   * Initializes the daemon with the provided configuration.
   * Performs an initial project scan to populate the layout and graph.
   *
   * @param cfg The project-specific configuration settings.
   */
  explicit Daemon(const Config& cfg);

  /**
   * Ensures all background threads are stopped and joined before destruction.
   */
  ~Daemon();

  /**
   * Starts the background watcher and builder threads.
   * This method returns immediately after the threads are launched.
   *
   * @return 0 on successful start, non-zero otherwise.
   */
  int run();

  /**
   * Gracefully shuts down the background threads and the build queue.
   */
  void stop();

 private:
  /**
   * Re-scans the filesystem to discover targets and update the dependency
   * graph. Thread-safe: locks the internal mutex to prevent reading stale
   * layout data.
   */
  void update_pl();

  /**
   * Triggers a full project rebuild and re-discovery.
   * @return The exit code of the underlying build command.
   */
  int rebuild_all();

  /**
   * Executes a build based on specific changed files.
   * @param task A batch of file events and build flags from the queue.
   * @return The exit code of the underlying build command.
   */
  int rebuild_changed(BuildQueue::Task& task);

  Config cfg_;
  ProjectLayout pl_;
  BuildQueue build_queue_;
  TargetGraph graph_;

  std::mutex mtx_;
  std::jthread watcher_thread_;
  std::jthread builder_thread_;
};

}  // namespace daemonmake

#endif