cmake_minimum_required(VERSION 3.14)

project(Qiti LANGUAGES CXX)

if(NOT APPLE)
    message(FATAL_ERROR “Only macOS builds are supported”)
endif()

set(ARCHS "arm64;x86_64") # Universal Binary

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -D_DEBUG")

# add_compile_options(”$<$CONFIG:Debug:-gdwarf-with-dsym>”)

# Generate the function wrapper file
# add_custom_command(TARGET qiti_lib
#     PRE_BUILD
#     COMMAND ${CMAKE_COMMAND} -E echo "Running pre-build Python script for qiti_lib"
#     # COMMAND /bin/sh ${CMAKE_CURRENT_SOURCE_DIR}/scripts/prebuild.sh
#     COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/prebuild.py
#     COMMENT "Executing pre-build tasks for qiti_lib via Python"
# )

set(SOURCES
    "source/qiti_utils.hpp"
    "source/qiti_utils.cpp"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/source PREFIX "Source" FILES ${SOURCES})

add_library(qiti_lib STATIC ${SOURCES})

set_target_properties(qiti_lib PROPERTIES
    OSX_ARCHITECTURES "${ARCHS}"
    XCODE_GENERATE_SCHEME ON
)

target_include_directories(qiti_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_compile_options(qiti_lib 
INTERFACE
    "-finstrument-functions"
    "-fno-omit-frame-pointer"
)

# =========================
#       Unit Testing
# =========================

include(FetchContent)
    FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(qiti_tests
    "Tests/qiti_test_macros.hpp"
    "Tests/test_qiti_utils.cpp"
)

set_target_properties(qiti_tests PROPERTIES
    OSX_ARCHITECTURES "${ARCHS}"
    XCODE_GENERATE_SCHEME ON
)

target_link_libraries(qiti_tests PRIVATE
    qiti_lib
    Catch2::Catch2WithMain
)

target_compile_options(qiti_tests PRIVATE
    "-Werror"
    "-Wpedantic"
)

enable_testing()

set_target_properties(Catch2 PROPERTIES 
    OSX_ARCHITECTURES "${ARCHS}"
)

set_target_properties(Catch2WithMain PROPERTIES
    OSX_ARCHITECTURES "${ARCHS}"
)

set_target_properties(qiti_tests PROPERTIES
    OSX_ARCHITECTURES "${ARCHS}"
    XCODE_GENERATE_SCHEME ON
)

# add_test(NAME qiti_tests COMMAND qiti_tests)

# Load and use the .cmake file provided by Catch2
# https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md
# We have to manually provide the source directory here for now

include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)

# When running ctest, you must add a configuration flag, e.g. "ctest -C Debug"
catch_discover_tests(qiti_tests DISCOVERY_MODE PRE_TEST)