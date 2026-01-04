#ifndef DAEMONMAKE__DAEMONMAKE_BUILD_QUEUE
#define DAEMONMAKE__DAEMONMAKE_BUILD_QUEUE

#include <condition_variable>
#include <mutex>
#include <vector>

#include "daemonmake/file_watcher.hpp"

namespace daemonmake {

class BuildQueue {
 public:
  explicit BuildQueue(size_t capacity);

  struct Task {
    std::vector<FileEvent> events;
    bool full_rebuild{};

    bool requires_discovery() const {
      for (const auto& event : events) {
        if (event.type == FileEventType::Created ||
            event.type == FileEventType::Deleted)
          return true;
      }

      return false;
    }
  };

  void push_event(const FileEvent& event);
  Task pop_all_events();
  void shutdown();

 private:
  size_t capacity_;
  std::vector<FileEvent> events_;
  bool shutdown_{};

  std::mutex mtx_;
  std::condition_variable cv_not_full_;
  std::condition_variable cv_not_empty_;
};

}  // namespace daemonmake

#endif