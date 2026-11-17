add_executable(aether_core_tests
    core_tests.cpp
)

target_link_libraries(aether_core_tests PRIVATE aether_core)
target_compile_features(aether_core_tests PRIVATE cxx_std_17)

add_test(NAME aether_core_tests COMMAND aether_core_tests)
