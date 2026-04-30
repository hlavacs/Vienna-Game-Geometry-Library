FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.14.0 # or a later release
)

FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

add_executable(tests_vulkan
    vulkanTest.cpp
)
target_link_libraries(tests_vulkan PRIVATE Catch2::Catch2WithMain vgeo)
catch_discover_tests(tests_vulkan)

add_executable(test_cpu_shapes
    AaBoxTest.cpp
    CapsuleTest.cpp
)
target_link_libraries(test_cpu_shapes PRIVATE Catch2::Catch2WithMain vgeo TerathonMathLibrary)
catch_discover_tests(test_cpu_shapes)
