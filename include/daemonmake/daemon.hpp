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

class Daemon {
 public:
  explicit Daemon(const Config& cfg);
  ~Daemon();
  int run();
  void stop();

 private:
  void update_pl();
  int rebuild_all();
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