cmake_minimum_required(VERSION 3.26)
project(MiniMaxPlayer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -flto")
#set(CMAKE_CXX_FLAGS_DEBUG " -H")

include_directories(include)

file(GLOB_RECURSE SOURCES "src/*.cpp")
message(STATUS "Source files: ${SOURCES}")
add_executable(MiniMaxPlayer ${SOURCES})
target_link_libraries(MiniMaxPlayer PRIVATE ws2_32)