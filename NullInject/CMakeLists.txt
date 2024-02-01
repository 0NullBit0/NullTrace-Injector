cmake_minimum_required(VERSION 3.22.1)

set(ABI "x86_64") # armeabi-v7a arm64-v8a x86 x86_64

set(CMAKE_SYSTEM_NAME ANDROID) #
set(CMAKE_SYSTEM_VERSION 28) #
set(ANDROID_PLATFORM 28) #
set(ANDROID_ABI ${ABI})
############################## CHANGE TO YOUR PATH
set(ANDROID_NDK )#/ndk/25.2.9519653) # CHANGE TO YOUR NDK PATH
set(CMAKE_TOOLCHAIN_FILE )#ndk/25.2.9519653/build/cmake/android.toolchain.cmake) # CHANGE TO YOUR PATH
############################## CHANGE TO YOUR PATH
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/outputs/${ABI})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-fno-rtti -fno-exceptions -DNDEBUG -fvisibility=hidden -Wno-narrowing -fdeclspec -pthread -w -s -fexceptions -Wall -O3")


project(NullInject)



file(GLOB_RECURSE LOCAL_SRC_FILES "app/src/**")
file(GLOB_RECURSE LOCAL_INCLUDE_FILES "app/include/**")

add_subdirectory(${CMAKE_SOURCE_DIR}/NullTrace)

add_executable(NullInject
        ${LOCAL_SRC_FILES}
        ${LOCAL_INCLUDE_FILES}
)
target_link_libraries(NullInject NullTrace)
include_directories(${CMAKE_SOURCE_DIR}/app/include)



