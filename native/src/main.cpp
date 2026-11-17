#include "aether/http_server.hpp"
#include "aether/supervisor.hpp"
#include "aether/version.hpp"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

int main(int argc,char** argv) {
    std::string proc_root="/proc";
    std::string web_root="/data/adb/modules/aether/web";
    std::uint16_t port=8765;
    std::uint32_t interval_ms=1000;
    bool once=false;
    for(int i=1;i<argc;++i) {
        const std::string arg=argv[i];
        auto next=[&]() -> const char* {return (i+1<argc)?argv[++i]:nullptr;};
        if(arg=="--proc-root") {const auto v=next();if(!v){std::cerr<<"missing --proc-root value\n";return 2;}proc_root=v;}
        else if(arg=="--web-root") {const auto v=next();if(!v){std::cerr<<"missing --web-root value\n";return 2;}web_root=v;}
        else if(arg=="--port") {const auto v=next();if(!v){std::cerr<<"missing --port value\n";return 2;}const long n=std::strtol(v,nullptr,10);if(n<1||n>65535){std::cerr<<"invalid port\n";return 2;}port=static_cast<std::uint16_t>(n);}
        else if(arg=="--interval-ms") {const auto v=next();if(!v){std::cerr<<"missing --interval-ms value\n";return 2;}const long n=std::strtol(v,nullptr,10);if(n<100||n>60000){std::cerr<<"interval must be 100..60000 ms\n";return 2;}interval_ms=static_cast<std::uint32_t>(n);}
        else if(arg=="--once") once=true;
        else if(arg=="--version") {std::cout<<aether::kVersion<<"\n";return 0;}
        else {std::cerr<<"unknown argument: "<<arg<<"\n";return 2;}
    }
    aether::Supervisor supervisor(proc_root);
    supervisor.sample_once();
    supervisor.tick();
    if(once) {
        std::cout<<supervisor.status_json()<<"\n"<<supervisor.memory_json()<<"\n";
        return 0;
    }
    aether::HttpServer server(web_root);
    std::thread http([&server,&supervisor,port]{
        const int result=server.run(supervisor,port);
        if(result!=0) std::cerr<<"AETHER HTTP server stopped: "<<result<<"\n";
    });
    (void)http.detach();
    std::cout<<"AETHER Serenity "<<aether::kVersion<<" listening on 127.0.0.1:"<<port<<"\n";
    for(;;) {
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
        supervisor.sample_once();
        supervisor.tick();
    }
}
