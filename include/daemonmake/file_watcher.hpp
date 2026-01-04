#ifndef DAEMONMAKE__DAEMONMAKE_FILE_WATCHER
#define DAEMONMAKE__DAEMONMAKE_FILE_WATCHER

#include <filesystem>
#include <unordered_map>
#include <vector>

namespace daemonmake {

enum struct FileEventType { Created, Modified, Deleted, Overflow };

struct FileEvent {
  std::filesystem::path path;
  FileEventType type;
};

class FileWatcher {
 public:
  explicit FileWatcher(const std::filesystem::path& project_root);
  ~FileWatcher();

  FileWatcher(const FileWatcher&) = delete;
  FileWatcher& operator=(const FileWatcher&) = delete;

  FileWatcher(FileWatcher&&) noexcept;
  FileWatcher& operator=(FileWatcher&&) noexcept;

  std::vector<FileEvent> wait_for_events();

 private:
  void add_watch(const std::filesystem::path& dir);

  int inotify_fd_;
  std::filesystem::path project_root_;
  std::unordered_map<int, std::filesystem::path> wd_to_path_;
};

}  // namespace daemonmake

#endif