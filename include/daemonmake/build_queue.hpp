#ifndef DAEMONMAKE__DAEMONMAKE_BUILD_QUEUE
#define DAEMONMAKE__DAEMONMAKE_BUILD_QUEUE

#include <condition_variable>
#include <filesystem>
#include <map>
#include <mutex>
#include <stop_token>
#include <vector>

#include "daemonmake/file_watcher.hpp"

namespace daemonmake {

/**
 * A thread-safe, debouncing priority queue for file system events.
 * 
 * Manages the synchronization between the file watcher (producer) and the
 * build engine (consumer), collapsing redundant events and delaying
 * processing until activity settles.
 */
class BuildQueue {
 public:
  /**
   * @param capacity Maximum number of unique file paths allowed in the queue.
   */
  explicit BuildQueue(size_t capacity);

  /**
   * Represents a batch of work to be processed by the builder.
   */
  struct Task {
    std::map<std::filesystem::path, FileEventType> events;
    bool full_rebuild{};

    /**
     * Checks if any event in the task requires re-running project discovery.
     * 
     * @return True if files were created or deleted.
     */
    bool requires_discovery() const {
      for (const auto& [path, type] : events) {
        if (type == FileEventType::Created || type == FileEventType::Deleted)
          return true;
      }

      return false;
    }
  };

  /**
   * Adds a file event to the queue, folding redundant events into one.
   * 
   * Blocks if the queue is at capacity. If a file is created and then
   * deleted before the queue is popped, the events are cancelled out.
   * 
   * @param event The file system event detected by the watcher.
   */
  void push_event(const FileEvent& event);

  /**
   * Waits for events and returns a batch of work after a debounce period.
   * 
   * This method blocks until events are available, then continues to wait
   * for a period of silence (debounce) to ensure multi-file operations
   * (like git checkouts) are captured in a single task.
   * 
   * @param token A stop_token to interrupt the wait for graceful shutdown.
   * @return A Task containing deduped events and build requirements.
   */
  Task pop_all_events(const std::stop_token& token);

  /**
   * Signals the queue to stop accepting events and wakes all waiting threads.
   */
  void shutdown();

 private:
  size_t capacity_;
  std::map<std::filesystem::path, FileEventType> events_{};
  bool needs_full_rebuild_{};
  std::chrono::steady_clock::time_point last_event_pushed_{};
  bool shutdown_{};

  std::mutex mtx_;
  std::condition_variable_any cv_not_full_;
  std::condition_variable_any cv_not_empty_;
};

}  // namespace daemonmake

#endif