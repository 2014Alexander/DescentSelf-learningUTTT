cmake_minimum_required(VERSION 3.26)
project(GeneralTestingSystem)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -flto")

include_directories(include)
file(GLOB_RECURSE SOURCES "src/*.cpp")
message(STATUS "Source files: ${SOURCES}")

add_executable(GeneralTestingSystem ${SOURCES})
target_link_libraries(GeneralTestingSystem ws2_32)