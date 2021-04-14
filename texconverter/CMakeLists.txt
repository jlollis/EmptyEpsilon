cmake_minimum_required(VERSION 3.8)
project(texconverter VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ISA_SSE2 ON CACHE BOOL "" FORCE)
add_subdirectory(astc-encoder EXCLUDE_FROM_ALL)

add_executable(texconverter repackTextures.cpp)
target_link_libraries(texconverter PUBLIC astcenc-sse2-static)

install(TARGETS texconverter)

