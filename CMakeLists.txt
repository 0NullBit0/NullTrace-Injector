cmake_minimum_required(VERSION 3.2.2)

set(CMAKE_SYSTEM_NAME ANDROID)
set(CMAKE_SYSTEM_VERSION 29)
set(ANDROID_PLATFORM 29)

set(ANDROID_NDK $ENV{NDK})
set(CMAKE_TOOLCHAIN_FILE $ENV{NDK}/build/cmake/android.toolchain.cmake)

project("NullTrace-Android-Injector" VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-fno-rtti -O3")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build_out/${CMAKE_ANDROID_ARCH_ABI})

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(NullElf)
add_subdirectory(NullTrace)

add_executable(NullTrace-Injector main.cpp)
target_link_libraries(NullTrace-Injector NullElf NullTrace)


