#include "aether/http_server.hpp"
#include <utility>
#include <cerrno>
#include <fstream>
#include <sstream>
#include <string>
#if defined(__unix__) || defined(__ANDROID__)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace aether {
namespace {
std::string read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return {};
    std::ostringstream out;
    out << in.rdbuf();
    return out.str();
}
std::string response(const std::string& status, const std::string& type, const std::string& body) {
    std::ostringstream out;
    out << "HTTP/1.1 " << status << "\r\nContent-Type: " << type
        << "\r\nContent-Length: " << body.size()
        << "\r\nCache-Control: no-store\r\nConnection: close"
        << "\r\nX-Content-Type-Options: nosniff\r\n\r\n" << body;
    return out.str();
}
std::string route(Supervisor& supervisor, const std::string& path,
                  const std::string& web_root, std::string& type, std::string& status) {
    type = "application/json; charset=utf-8";
    status = "200 OK";
    if (path == "/api/v1/status") return supervisor.status_json();
    if (path == "/api/v1/memory") return supervisor.memory_json();
    if (path == "/api/v1/processes") return supervisor.processes_json();
    if (path == "/api/v1/events") return supervisor.events_json();
    if (path == "/" || path == "/index.html") {
        type = "text/html; charset=utf-8";
        const auto page = read_file(web_root + "/index.html");
        if (page.empty()) {
            status = "404 Not Found";
            return "WebUI file not found";
        }
        return page;
    }
    status = "404 Not Found";
    return "{\"error\":\"not_found\"}";
}
} // namespace

HttpServer::HttpServer(std::string root) : web_root_(std::move(root)) {}

int HttpServer::run(Supervisor& supervisor, std::uint16_t port) {
#if defined(__unix__) || defined(__ANDROID__)
    const int server = ::socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) return errno;
    int yes = 1;
    (void)::setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (::bind(server, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        const int error = errno;
        ::close(server);
        return error;
    }
    if (::listen(server, 16) < 0) {
        const int error = errno;
        ::close(server);
        return error;
    }
    for (;;) {
        const int client = ::accept(server, nullptr, nullptr);
        if (client < 0) {
            if (errno == EINTR) continue;
            break;
        }
        char buffer[4096]{};
        const auto received = ::recv(client, buffer, sizeof(buffer) - 1U, 0);
        std::string method;
        std::string path;
        if (received > 0) {
            std::istringstream request(std::string(buffer, static_cast<std::size_t>(received)));
            request >> method >> path;
        }
        std::string type;
        std::string status;
        std::string body;
        if (method == "GET") {
            body = route(supervisor, path, web_root_, type, status);
        } else {
            type = "application/json; charset=utf-8";
            status = "405 Method Not Allowed";
            body = "{\"error\":\"method_not_allowed\"}";
        }
        const auto bytes = response(status, type, body);
        std::size_t sent = 0;
        while (sent < bytes.size()) {
#ifdef MSG_NOSIGNAL
            const auto count = ::send(client, bytes.data() + sent, bytes.size() - sent, MSG_NOSIGNAL);
#else
            const auto count = ::send(client, bytes.data() + sent, bytes.size() - sent, 0);
#endif
            if (count <= 0) break;
            sent += static_cast<std::size_t>(count);
        }
        ::close(client);
    }
    ::close(server);
    return errno;
#else
    (void)supervisor;
    (void)port;
    return -1;
#endif
}
} // namespace aether
