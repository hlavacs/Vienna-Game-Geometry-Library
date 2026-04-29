cmake_minimum_required(VERSION 3.30)

project(
    "Vienna-Game-Geometry-Library"
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# Dependencies
include(FetchContent)

FetchContent_Declare(
    terathon_math_library
    GIT_REPOSITORY https://github.com/EricLengyel/Terathon-Math-Library.git
    GIT_TAG        40fcfa252313af6c37b89cfe09c530ac6174a06b
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(terathon_math_library)

file(GLOB TERATHON_SOURCES ${terathon_math_library_SOURCE_DIR}/*.cpp)
add_library(TerathonMathLibrary STATIC ${TERATHON_SOURCES})
target_include_directories(TerathonMathLibrary PUBLIC ${terathon_math_library_SOURCE_DIR})

find_package(Vulkan REQUIRED)


# Library
add_library(vgeo STATIC
    vgeo/internal/ConvexHullBuilder.cpp
    vgeo/internal/cpu/ConvexHull.cpp
    vgeo/internal/gpu/GpuBackend.cpp
)
target_compile_features(vgeo PUBLIC cxx_std_26)
target_include_directories(vgeo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(vgeo PRIVATE TerathonMathLibrary)

target_link_libraries(vgeo PUBLIC Vulkan::Vulkan)

# Examples and tests
if(PROJECT_IS_TOP_LEVEL)
    add_executable(demo examples/demo.cpp)
    set_target_properties(demo PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    target_link_libraries(demo PRIVATE vgeo)

    if(WIN32)
        target_link_libraries(demo PRIVATE stdc++exp)
        target_link_options(demo PRIVATE -static)
    endif()

    enable_testing()
    add_subdirectory(tests)

endif()
