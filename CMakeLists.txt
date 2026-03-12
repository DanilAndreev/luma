cmake_minimum_required(VERSION 3.28)
project(luma)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external/assimp)

set(HEADER_FILES 
        src/pch.h
        src/Helpers.h
        src/Scene.h
        src/SceneLoader.h
        )

set(SOURCE_FILES
        src/main.cpp
        src/Scene.h
        src/SceneLoader.cpp
        )

set(HLSL_SOURCE_FILES
        shaders/src/unity.hlsl)

add_executable(luma WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${HLSL_SOURCE_FILES})
set_source_files_properties(${HLSL_SOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
target_precompile_headers(luma PRIVATE src/pch.h)
target_link_libraries(luma PUBLIC d3d11.lib)
target_link_libraries(luma PUBLIC dxgi.lib)

set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_INSTALL OFF)
target_link_libraries(luma PRIVATE assimp::assimp)

add_custom_command(TARGET luma POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:luma>
                   $<TARGET_RUNTIME_DLLS:luma> USES_TERMINAL COMMAND_EXPAND_LISTS)
