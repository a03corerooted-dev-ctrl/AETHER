#pragma once
#include "aether/supervisor.hpp"
#include <cstdint>
#include <string>
namespace aether {
class HttpServer final {
public:
    explicit HttpServer(std::string web_root);
    [[nodiscard]] int run(Supervisor& supervisor,std::uint16_t port);
private:
    std::string web_root_;
};
} // namespace aether
