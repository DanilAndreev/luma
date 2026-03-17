cmake_minimum_required(VERSION 3.28)
project(luma)

set(CMAKE_CXX_STANDARD 20)

find_package(Python3 REQUIRED)

add_subdirectory(external/assimp)

set(HEADER_FILES 
        src/pch.h
        src/Globals.h
        src/Helpers.h
        src/Scene.h
        src/Camera.h
        src/SceneLoader.h
        src/ShaderManager.h
        src/Input/InputDeviceState.h
        src/SceneRenderer.h
        src/Win/BaseWindow.h
        )

set(SOURCE_FILES
        src/main.cpp
        src/stbImpl.cpp
        src/Globlas.cpp
        src/Scene.h
        src/Camera.cpp
        src/SceneLoader.cpp
        src/ShaderManager.cpp
        src/Input/InputDeviceState.cpp
        src/SceneRenderer.cpp
        src/Input/InputManager.cpp
        )

set(HLSL_SOURCE_FILES
        shaders/hlsl/HLSLCPPCompat.hlsli
        shaders/hlsl/ShaderTypes.hlsli
        shaders/hlsl/vertexSemantics.hlsli

        shaders/hlsl/vertex.hlsl
        shaders/hlsl/unity.hlsl
)

add_executable(luma WIN32 ${HEADER_FILES} ${SOURCE_FILES} ${HLSL_SOURCE_FILES})
set_source_files_properties(${HLSL_SOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
target_precompile_headers(luma PRIVATE src/pch.h)
target_link_libraries(luma PUBLIC d3d11.lib)
target_link_libraries(luma PUBLIC dxgi.lib)
target_link_libraries(luma PUBLIC dxguid.lib)
target_include_directories(luma PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(luma PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
target_include_directories(luma PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/stb)
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_INSTALL OFF)
target_link_libraries(luma PRIVATE assimp::assimp)

add_custom_command(TARGET luma POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:luma> $<TARGET_RUNTIME_DLLS:luma>
                   USES_TERMINAL COMMAND_EXPAND_LISTS)

add_custom_command(TARGET luma POST_BUILD
                   COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/shaders/compile.py -o $<TARGET_FILE_DIR:luma>/sm
                   USES_TERMINAL COMMAND_EXPAND_LISTS)
