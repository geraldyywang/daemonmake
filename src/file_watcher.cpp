#include "daemonmake/file_watcher.hpp"

#include <poll.h>
#include <sys/inotify.h>
#include <unistd.h>

namespace daemonmake {

namespace fs = std::filesystem;

FileWatcher::FileWatcher(const fs::path& project_root)
    : project_root_{project_root}, inotify_fd_{inotify_init()} {
  if (inotify_fd_ < 0) throw std::runtime_error("Failed to initialize inotify");

  if (fs::exists(project_root_)) {
    add_watch(project_root_);
    for (const auto& entry : fs::recursive_directory_iterator(project_root_)) {
      if (!fs::is_directory(entry)) continue;
      add_watch(entry.path());
    }
  }
}

FileWatcher::~FileWatcher() {
  if (inotify_fd_ >= 0) {
    close(inotify_fd_);
  }
}

FileWatcher::FileWatcher(FileWatcher&& other) noexcept
    : project_root_{std::move(other.project_root_)},
      inotify_fd_{std::exchange(other.inotify_fd_, -1)},
      wd_to_path_{std::move(other.wd_to_path_)} {
  other.inotify_fd_ = -1;
}

FileWatcher& FileWatcher::operator=(FileWatcher&& other) noexcept {
  if (this == &other) return *this;

  if (inotify_fd_ >= 0) {
    close(inotify_fd_);
  }

  project_root_ = std::move(other.project_root_);
  inotify_fd_ = std::exchange(other.inotify_fd_, -1);
  wd_to_path_ = std::move(other.wd_to_path_);

  return *this;
}

std::vector<FileEvent> FileWatcher::wait_for_events() {
  pollfd pfd{};
  pfd.fd = inotify_fd_;
  pfd.events = POLLIN;

  int ret{poll(&pfd, 1, 500)};
  if (ret <= 0) return {};

  constexpr size_t EVENT_SIZE{sizeof(inotify_event)};
  constexpr size_t NAME_LEN_ESTIMATE{16};
  constexpr size_t MAX_BATCH_SIZE{1024};
  constexpr size_t BUF_LEN{MAX_BATCH_SIZE * (EVENT_SIZE + NAME_LEN_ESTIMATE)};

  alignas(inotify_event) std::array<char, BUF_LEN> event_buffer;

  ssize_t bytes_read{read(inotify_fd_, event_buffer.data(), BUF_LEN)};
  if (bytes_read < 0) return {};

  std::vector<FileEvent> events;
  ssize_t i{};
  while (i < bytes_read) {
    inotify_event* event{(inotify_event*)&event_buffer[i]};

    if (event->len) {
      auto it = wd_to_path_.find(event->wd);
      if (it != wd_to_path_.end()) {
        const fs::path& dir{it->second};
        const fs::path full_path{dir / event->name};

        if (event->mask & (IN_CREATE | IN_MOVED_TO) &&
            fs::is_directory(full_path))
          add_watch(full_path);

        // TODO: handle wd clear? Not handling it for now as I would need to
        // create a path -> wd map

        // TODO: handle overflow

        FileEventType type{FileEventType::Modified};
        if (event->mask & IN_CREATE)
          type = FileEventType::Created;
        else if (event->mask & IN_DELETE)
          type = FileEventType::Deleted;

        if (full_path.extension() != ".swp" &&
            full_path.extension() != ".tmp") {
          events.push_back({full_path, type});
        }
      }
    }

    i += EVENT_SIZE + event->len;
  }

  return events;
}

void FileWatcher::add_watch(const fs::path& dir) {
  int wd{inotify_add_watch(inotify_fd_, dir.c_str(),
                           IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_TO)};
  if (wd < 0) return;

  wd_to_path_[wd] = dir;
}

}  // namespace daemonmake
