#ifndef DAEMONMAKE__DAEMONMAKE_FILE_WATCHER
#define DAEMONMAKE__DAEMONMAKE_FILE_WATCHER

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace daemonmake {

/**
 * High-level categories for filesystem changes.
 * Overflow indicates the kernel event queue was exceeded or a watched
 * directory was moved/deleted, necessitating a full project re-scan.
 */
enum struct FileEventType { Created, Modified, Deleted, Overflow };

/**
 * A simplified representation of a filesystem change event.
 */
struct FileEvent {
  std::filesystem::path path;
  FileEventType type;
};

/**
 * Linux-specific file watcher using the inotify API.
 * Monitored directories are watched recursively. This class handles
 * the complexities of inotify, such as mapping watch descriptors back
 * to paths and updating watches when new directories are created.
 */
class FileWatcher {
 public:
  /**
   * Initializes inotify and establishes recursive watches on all roots.
   * 
   * @param roots A list of directory paths to monitor.
   * @throws std::runtime_error If inotify_init fails.
   */
  explicit FileWatcher(const std::vector<std::filesystem::path>& roots);

  /**
   * Closes the inotify file descriptor and stops all watches.
   */
  ~FileWatcher();

  // Non-copyable due to file descriptor ownership.
  FileWatcher(const FileWatcher&) = delete;
  FileWatcher& operator=(const FileWatcher&) = delete;

  FileWatcher(FileWatcher&&) noexcept;
  FileWatcher& operator=(FileWatcher&&) noexcept;

  /**
   * Blocks for a short duration to wait for filesystem events.
   * 
   * @return A vector of events that occurred. Returns empty if a timeout 
   * occurs or no events were pending.
   */
  std::vector<FileEvent> wait_for_events();

 private:
 /**
   * Registers a single directory with the inotify instance.
   * 
   * @param dir The directory path to watch.
   */
  void add_watch(const std::filesystem::path& dir);

  int inotify_fd_;
  std::vector<std::filesystem::path> roots_;
  std::unordered_map<int, std::filesystem::path> wd_to_path_;
};

}  // namespace daemonmake

#endif