cmake_minimum_required(VERSION 3.26)
project(DescentPlayer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
#set(CMAKE_CXX_FLAGS_DEBUG " -H")

include_directories(include)
include_directories(C:/dev/Python310/Include)         # Python заголовки
include_directories(C:/dev/Python310/Lib/site-packages/pybind11/include)
link_directories(C:/dev/Python310/libs)

file(GLOB_RECURSE SOURCES "src/*.cpp")
message(STATUS "Source files: ${SOURCES}")
add_executable(DescentPlayer ${SOURCES})
target_link_libraries(DescentPlayer PRIVATE ws2_32)
target_link_libraries(DescentPlayer PRIVATE python310)
target_precompile_headers(DescentPlayer PRIVATE
        include/shared_memory/SharedMemory.h
        include/structures/robin_lib/robin_set.h
        include/structures/robin_lib/robin_map.h
)
#target_link_options(DescentPlayer PRIVATE "-Wl,--stack,8388608")