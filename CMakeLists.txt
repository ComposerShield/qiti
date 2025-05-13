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

set(SOURCES
    "include/qiti_include.hpp"
    "source/qiti_API.hpp"
    "source/qiti_FunctionCallData_Impl.hpp"
    "source/qiti_FunctionCallData.hpp"
    "source/qiti_FunctionCallData.cpp"
    "source/qiti_FunctionData_Impl.hpp"
    "source/qiti_FunctionData.hpp"
    "source/qiti_FunctionData.cpp"
    "source/qiti_instrument.hpp"
    "source/qiti_instrument.cpp"
    "source/qiti_LeakSanitizer.hpp"
    "source/qiti_LeakSanitizer.cpp"
    "source/qiti_profile.hpp"
    "source/qiti_profile.cpp"
    "source/qiti_ReentrantSharedMutex.hpp"
    "source/qiti_ScopedNoHeapAllocations.hpp"
    "source/qiti_ThreadSanitizer.hpp"
    "source/qiti_ThreadSanitizer.cpp"
    "source/qiti_TypeData_Impl.hpp"
    "source/qiti_TypeData.hpp"
    "source/qiti_TypeData.cpp"
    "source/qiti_utils.hpp"
    "source/qiti_utils.cpp"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "lib" FILES ${SOURCES})

add_library(qiti_lib SHARED ${SOURCES})

set_target_properties(qiti_lib PROPERTIES
    OSX_ARCHITECTURES "${ARCHS}"
    XCODE_GENERATE_SCHEME ON
)

target_include_directories(qiti_lib 
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/source
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(qiti_lib 
PRIVATE
    # dylib symbol visibility
    "-fvisibility=hidden"
    "-fvisibility-inlines-hidden"
    # warnings/errors
    "-Werror"
    "-Wall"
    "-Wextra"
    "-Wpedantic"
    "-Wconversion"
    "-Wsign-conversion" 
    "-Wshadow"
    "-Wold-style-cast"
    "-Wnon-virtual-dtor"
    "-Woverloaded-virtual"
    "-Wnull-dereference"
    "-Wdouble-promotion"
    "-Wformat=2"
    "-Wimplicit-fallthrough"
INTERFACE
    "-finstrument-functions"
    "-fno-omit-frame-pointer"
    "-fsanitize=thread"
    $<$<CONFIG:Debug>:-fsanitize=thread -g -O0>
)

target_compile_definitions(qiti_lib
PRIVATE
    QITI_DYLIB=1
)

target_link_options(qiti_lib
INTERFACE
    $<$<CONFIG:Debug>:-fsanitize=thread>
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
    "Tests/test_qiti_FunctionCallData.cpp"
    "Tests/test_qiti_FunctionData.cpp"
    "Tests/test_qiti_instrument.cpp"
    "Tests/test_qiti_profile.cpp"
    "Tests/test_qiti_LeakSanitizer.cpp"
    "Tests/test_qiti_ScopedNoHeapAllocations.cpp"
    "Tests/test_qiti_ThreadSanitizer.cpp"
    "Tests/test_qiti_TypeData.cpp"
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

target_include_directories(qiti_tests 
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/source
)

# Load and use the .cmake file provided by Catch2
# https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md
# We have to manually provide the source directory here for now

include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)

# When running ctest, you must add a configuration flag, e.g. "ctest -C Debug"
catch_discover_tests(qiti_tests DISCOVERY_MODE PRE_TEST)