cmake_minimum_required(VERSION 4.1)
project(luma)

set(CMAKE_CXX_STANDARD 20)

add_executable(luma WIN32 main.cpp)
target_link_libraries(luma PUBLIC d3d11.lib)
target_link_libraries(luma PUBLIC dxgi.lib)
