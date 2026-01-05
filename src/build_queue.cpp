#include "daemonmake/build_queue.hpp"

namespace daemonmake {

using clock = std::chrono::steady_clock;
using namespace std::chrono_literals;

BuildQueue::BuildQueue(size_t capacity) : capacity_{capacity} {}

void BuildQueue::push_event(const FileEvent& event) {
  std::unique_lock<std::mutex> lock{mtx_};

  cv_not_full_.wait(lock,
                    [this] { return events_.size() < capacity_ || shutdown_; });
  if (shutdown_) return;

  if (event.type == FileEventType::Overflow)
    needs_full_rebuild_ = true;
  else if (!events_.count(event.path) ||
           events_[event.path] == FileEventType::Modified)
    events_[event.path] = event.type;
  else if (events_[event.path] == FileEventType::Created &&
           event.type == FileEventType::Deleted)
    events_.erase(event.path);

  last_event_pushed_ = clock::now();
  cv_not_empty_.notify_one();
}

BuildQueue::Task BuildQueue::pop_all_events(const std::stop_token& token) {
  std::unique_lock<std::mutex> lock{mtx_};

  cv_not_empty_.wait(lock, token, [this] {
    return !events_.empty() || shutdown_ || needs_full_rebuild_;
  });
  if ((shutdown_ || token.stop_requested()) && events_.empty() &&
      !needs_full_rebuild_)
    return {};

  // For debouncing and trying to group more events
  const auto debounce_timeout{1000ms};

  while (!shutdown_ && !token.stop_requested() && !needs_full_rebuild_) {
    const auto last_push_before_sleep{last_event_pushed_};
    const auto deadline{last_event_pushed_ + debounce_timeout};
    cv_not_empty_.wait_until(
        lock, token, deadline, [this, last_push_before_sleep] {
          return shutdown_ || last_event_pushed_ != last_push_before_sleep;
        });
    if (last_push_before_sleep == last_event_pushed_ &&
        clock::now() >= deadline)
      break;
  }

  Task task{std::move(events_), needs_full_rebuild_};
  events_.clear();
  needs_full_rebuild_ = false;

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