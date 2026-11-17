#include "aether/event_log.hpp"
#include <utility>
#include <algorithm>
#include <chrono>
namespace aether {
EventLog::EventLog(std::size_t capacity):capacity_(std::max<std::size_t>(1U,capacity)){}
void EventLog::push(std::string level,std::string category,std::string message) {
    const auto now=std::chrono::system_clock::now().time_since_epoch();
    const auto ms=static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
    std::lock_guard<std::mutex> lock(mutex_);
    events_.push_back(Event{next_sequence_++,ms,std::move(level),std::move(category),std::move(message)});
    while(events_.size()>capacity_) events_.pop_front();
}
std::vector<Event> EventLog::snapshot(std::size_t limit) const {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto count=std::min(limit,events_.size());
    return {events_.end()-static_cast<std::ptrdiff_t>(count),events_.end()};
}
} // namespace aether
