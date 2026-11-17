add_library(aether_core STATIC
    src/engine.cpp
)

target_include_directories(aether_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(aether_core PUBLIC cxx_std_17)

if(AETHER_STRICT_WARNINGS)
    if(MSVC)
        target_compile_options(aether_core PRIVATE /W4 /WX)
    else()
        target_compile_options(aether_core PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion
        )
    endif()
endif()

set_target_properties(aether_core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME "aether_core"
)
