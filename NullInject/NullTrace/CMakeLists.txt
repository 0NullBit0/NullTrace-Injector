



file(GLOB_RECURSE LOCAL_SRC_FILES "app/src/**")
file(GLOB_RECURSE LOCAL_INCLUDE_FILES "app/include/**")

add_subdirectory(NullElf)

add_library(NullTrace STATIC
        ${LOCAL_SRC_FILES}
        ${LOCAL_INCLUDE_FILES}
)
target_link_libraries(NullTrace NullElf)
include_directories(app/include)



