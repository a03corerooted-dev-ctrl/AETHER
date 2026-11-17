cmake_minimum_required(VERSION 3.22)

project(AETHER
    VERSION 0.1.0
    DESCRIPTION "AETHER Android memory-management engine foundation"
    LANGUAGES CXX
)

option(AETHER_BUILD_TESTS "Build host-side foundation tests" ON)
option(AETHER_STRICT_WARNINGS "Enable strict compiler warnings" ON)

if(ANDROID)
    if(NOT ANDROID_ABI STREQUAL "arm64-v8a")
        message(FATAL_ERROR
            "AETHER Android builds support arm64-v8a only; received: ${ANDROID_ABI}")
    endif()

    set(AETHER_BUILD_TESTS OFF CACHE BOOL
        "Host-side tests are not built for Android targets" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(native)

if(AETHER_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(native/tests)
endif()
