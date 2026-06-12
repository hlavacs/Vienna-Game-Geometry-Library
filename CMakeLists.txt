cmake_minimum_required(VERSION 3.30)

project(
    "Vienna-Game-Geometry-Library"
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Dependencies
include(FetchContent)

option(VGEO_DOUBLE_PRECISION "Use double precision floating point" OFF)

set(TERATHON_MATH_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/terathon_math_library)

file(GLOB TERATHON_SOURCES ${TERATHON_MATH_LIBRARY_DIR}/*.cpp)
add_library(TerathonMathLibrary STATIC ${TERATHON_SOURCES})
target_include_directories(TerathonMathLibrary PUBLIC ${TERATHON_MATH_LIBRARY_DIR})
if(VGEO_DOUBLE_PRECISION)
    target_compile_definitions(TerathonMathLibrary PUBLIC VGEO_DOUBLE_PRECISION)
endif()

find_package(Vulkan REQUIRED)


# Library
add_library(vgeo STATIC
    vgeo/internal/ConvexHullBuilder.cpp
    vgeo/internal/gpu/VulkanHandler.cpp
)
target_compile_features(vgeo PUBLIC cxx_std_26)
target_include_directories(vgeo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vgeo PUBLIC TerathonMathLibrary)

target_link_libraries(vgeo PUBLIC Vulkan::Vulkan)

# Examples and tests
if(PROJECT_IS_TOP_LEVEL)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG        6.0
        EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(raylib)

    add_executable(demo examples/demo.cpp)
    set_target_properties(demo PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    target_link_libraries(demo PRIVATE vgeo)

    add_executable(visualizer examples/visualizer.cpp)
    set_target_properties(visualizer PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    target_link_libraries(visualizer PRIVATE vgeo raylib)

    if(WIN32)
        target_link_libraries(demo PRIVATE stdc++exp)
        target_link_options(demo PRIVATE -static)
    endif()

    enable_testing()
    add_subdirectory(tests)

endif()
