#include "daemonmake/build_queue.hpp"

namespace daemonmake {

BuildQueue::BuildQueue(size_t capacity) : capacity_{capacity} {}

void BuildQueue::push_event(const FileEvent& event) {
  std::unique_lock<std::mutex> lock{mtx_};

  cv_not_full_.wait(lock, [this] { return events_.size() < capacity_ || shutdown_; });
  if (shutdown_) return;

  events_.push_back(event);
  cv_not_empty_.notify_one();
}

BuildQueue::Task BuildQueue::pop_all_events() {
  std::unique_lock<std::mutex> lock{mtx_};

  cv_not_empty_.wait(lock, [this] { return !events_.empty() || shutdown_; });
  if (shutdown_ && events_.empty()) return {};

  Task task{};
  task.events = std::move(events_);
  events_.clear();

  cv_not_full_.notify_all();
  return task;
}

void BuildQueue::shutdown() {
  {
    std::scoped_lock<std::mutex> lock{mtx_};
    shutdown_ = true;
  }
  cv_not_full_.notify_all();
  cv_not_empty_.notify_all();
}

}  // namespace daemonmake