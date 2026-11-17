#pragma once
#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <vector>
namespace aether {
struct Event final { std::uint64_t sequence{0}; std::uint64_t timestamp_ms{0}; std::string level; std::string category; std::string message; };
class EventLog final {
public:
    explicit EventLog(std::size_t capacity=256);
    void push(std::string level,std::string category,std::string message);
    [[nodiscard]] std::vector<Event> snapshot(std::size_t limit=64) const;
private:
    std::size_t capacity_;
    mutable std::mutex mutex_;
    std::deque<Event> events_;
    std::uint64_t next_sequence_{1};
};
} // namespace aether
