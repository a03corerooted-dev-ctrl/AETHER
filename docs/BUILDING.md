# Building AETHER

## Requirements

- CMake 3.22 or newer.
- A C++17 compiler for host-side development.
- Android NDK with CMake toolchain support for Android ARM64 builds.

## Host build and tests

```sh
cmake -S . -B build -DAETHER_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The host build exists to test policy-independent native code. It is not an Android
module and must not be installed on a device.

## Android ARM64 cross-build

Linux/macOS shell example:

```sh
cmake -S . -B build-android \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-23 \
  -DAETHER_BUILD_TESTS=OFF
cmake --build build-android --parallel
```

The build system rejects Android ABIs other than `arm64-v8a`. The selected API
level is only a compilation baseline; it does not certify compatibility with
private system services on a device.

## Build validation still required

Before a device build can be called supported, CI and device testing must verify:
compiler/toolchain versions, linker output, Android runtime loading, SELinux
constraints, and the exact integration contract available on the target build.
