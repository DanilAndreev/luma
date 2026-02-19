cmake_minimum_required(VERSION 4.1)
project(luma)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external/assimp)

set(HEADER_FILES 
        src/Scene.h
        src/SceneLoader.h
        )

set(SOURCE_FILES
        src/main.cpp
        src/Scene.h
        src/SceneLoader.cpp
        )

add_executable(luma WIN32 ${HEADER_FILES} ${SOURCE_FILES})
target_precompile_headers(luma PRIVATE src/pch.h)
target_link_libraries(luma PUBLIC d3d11.lib)
target_link_libraries(luma PUBLIC dxgi.lib)

set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_INSTALL OFF)
target_link_libraries(luma PRIVATE assimp::assimp)