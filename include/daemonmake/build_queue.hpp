#ifndef DAEMONMAKE__DAEMONMAKE_BUILD_QUEUE
#define DAEMONMAKE__DAEMONMAKE_BUILD_QUEUE

#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <stop_token>
#include <map>
#include <vector>

#include "daemonmake/file_watcher.hpp"

namespace daemonmake {

class BuildQueue {
 public:
  explicit BuildQueue(size_t capacity);

  struct Task {
    std::map<std::filesystem::path, FileEventType> events;
    bool full_rebuild{};

    bool requires_discovery() const {
      for (const auto& [path, type] : events) {
        if (type == FileEventType::Created || type == FileEventType::Deleted)
          return true;
      }

      return false;
    }
  };

  void push_event(const FileEvent& event);
  Task pop_all_events(const std::stop_token& token);
  void shutdown();

 private:
  size_t capacity_;
  std::map<std::filesystem::path, FileEventType> events_ {};
  bool needs_full_rebuild_{};
  std::chrono::steady_clock::time_point last_event_pushed_{};
  bool shutdown_{};

  std::mutex mtx_;
  std::condition_variable_any cv_not_full_;
  std::condition_variable_any cv_not_empty_;
};

}  // namespace daemonmake

#endif