cmake_minimum_required(VERSION 3.22.1)




file(GLOB_RECURSE LOCAL_SRC_FILES "app/src/**")
file(GLOB_RECURSE LOCAL_INCLUDE_FILES "app/include/**")
add_library(NullElf STATIC
        ${LOCAL_SRC_FILES}
        ${LOCAL_INCLUDE_FILES}
)

include_directories(${CMAKE_SOURCE_DIR}/app/include)



