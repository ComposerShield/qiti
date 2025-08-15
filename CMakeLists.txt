cmake_minimum_required(VERSION 3.14)

set(QITI_VERSION "0.0.1")
project(Qiti 
    VERSION ${QITI_VERSION} 
    LANGUAGES C CXX
)

if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"))
    message(FATAL_ERROR "Only Clang or Apple Clang compilers are supported")
endif()


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optional ThreadSanitizer support
option(QITI_ENABLE_THREAD_SANITIZER "Enable ThreadSanitizer wrapper functionality" OFF)

# Check for unsupported ThreadSanitizer on Windows
if(QITI_ENABLE_THREAD_SANITIZER AND WIN32)
    message(FATAL_ERROR "ThreadSanitizer features are not supported on Windows. Please disable QITI_ENABLE_THREAD_SANITIZER.")
endif()

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
    "source/qiti_HotspotDetector.hpp"
    "source/qiti_HotspotDetector.cpp"
    "source/qiti_Instrument.hpp"
    "source/qiti_Instrument.cpp"
    "source/qiti_InstrumentHooks.cpp"
    "source/qiti_LeakSanitizer.hpp"
    "source/qiti_LeakSanitizer.cpp"
    "source/qiti_LockData.hpp"
    "source/qiti_LockData.cpp"
    "source/qiti_LockHooks.cpp"
    "source/qiti_LockHooks.hpp"
    "source/qiti_MallocHooks.hpp"
    "source/qiti_MallocHooks.cpp"
    "source/qiti_Profile.hpp"
    "source/qiti_Profile.cpp"
    "source/qiti_ExceptionHooks.cpp"
    "source/qiti_ScopedNoHeapAllocations.hpp"
    "source/qiti_ScopedQitiTest.hpp"
    "source/qiti_ScopedQitiTest.cpp"
    "source/qiti_ThreadSanitizer.hpp"
    "source/qiti_ThreadSanitizer.cpp"
    "source/qiti_TypeData_Impl.hpp"
    "source/qiti_TypeData.hpp"
    "source/qiti_TypeData.cpp"
    "source/qiti_Utils.hpp"
    "source/qiti_Utils.cpp"
)

set(EXAMPLE_SOURCES
    "example/source/qiti_example_include.hpp"
    "example/source/qiti_example.cpp"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "lib" FILES ${SOURCES} ${EXAMPLE_SOURCES})

# Public Qiti Library
add_library(qiti_lib SHARED ${SOURCES})

# Force client to compile this file
add_library(qiti_tests_client OBJECT "source/client/qiti_client_tsan_integration.cpp")

if(PROJECT_IS_TOP_LEVEL)
    # Internal example app used for testing
    if(WIN32)
        # Static library on Windows to avoid DLL export issues
        add_library(qiti_example_target STATIC ${EXAMPLE_SOURCES})
    else()
        add_library(qiti_example_target SHARED ${EXAMPLE_SOURCES})
    endif()

    target_link_libraries(qiti_example_target
    PRIVATE
        qiti_lib
    )
endif()

if(APPLE)
    set_target_properties(qiti_lib PROPERTIES
        VERSION     ${PROJECT_VERSION}
        XCODE_GENERATE_SCHEME ON
        INSTALL_NAME_DIR "@executable_path" # place next to executable within bundle
    )
    
    if(PROJECT_IS_TOP_LEVEL)
        set_target_properties(qiti_example_target PROPERTIES
            VERSION     ${PROJECT_VERSION}
            XCODE_GENERATE_SCHEME ON
        )
    endif()
endif()

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
    "-Werror" # upgrade warnings into erros
    "-Wall"
    "-Wconversion"
    "-Wdangling-else"
    "-Wdouble-promotion"
    "-Wextra"
    "-Wfloat-equal"
    "-Wfloat-conversion"
    "-Wformat=2"
    "-Wimplicit-fallthrough"
    "-Wmissing-include-dirs"
    "-Wnon-virtual-dtor"
    "-Wnull-dereference"
    "-Woverloaded-virtual"
    "-Wold-style-cast"
    "-Wpedantic"
    "-Wshadow"
    "-Wsign-conversion"
    "-Wstring-conversion"
    "-Wzero-as-null-pointer-constant"
INTERFACE
    # Instruments all visible functions not marked __attribute__((no_instrument_function))
    "-finstrument-functions"
    # Required to walk the stack from within our function hooks (__cyg_profile_func_enter/exit)
    "-fno-omit-frame-pointer"
    # Generate debug info
    "-g" 
)

# ThreadSanitizer-specific options (only when enabled)
if(QITI_ENABLE_THREAD_SANITIZER)
    target_compile_options(qiti_lib INTERFACE
        # Prevent inlining which breaks our ability to track memory with TSan
        "-fno-inline"
        # Enable Thread Sanitizer
        "-fsanitize=thread"
    )
    target_compile_definitions(qiti_lib PUBLIC QITI_ENABLE_THREAD_SANITIZER=1)
endif()

target_compile_definitions(qiti_lib
PRIVATE
    QITI_DYLIB=1
PUBLIC
    QITI_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    QITI_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    QITI_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    QITI_VERSION=\"${PROJECT_VERSION}\" # need the escape-quoted string for a valid C literal:
)

if(WIN32)
    # Do not define "min" and "max" via windows.h
    target_compile_definitions(qiti_lib PRIVATE NOMINMAX)
    # Link against Windows debugging libraries for symbol resolution
    target_link_libraries(qiti_lib PRIVATE dbghelp)
    # Put DLL in same directory as test executables
    set_target_properties(qiti_lib PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()

# ThreadSanitizer-specific link options (only when enabled)
if(QITI_ENABLE_THREAD_SANITIZER)
    target_link_options(qiti_lib INTERFACE "-fsanitize=thread")
endif()

# =========================
#       Unit Testing
# =========================

if(PROJECT_IS_TOP_LEVEL)
    include(FetchContent)
    
    # Fetch Catch2
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.5.0
    )
    FetchContent_MakeAvailable(Catch2)
    
    # Fetch GTest
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
    )
    
    # Force GTest to use shared runtime on Windows to match our project
    if(WIN32)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()
    
    FetchContent_MakeAvailable(googletest)

    # Test sources list
    if(WIN32)
        # Windows: Start with minimal test set for debugging
        set(TEST_SOURCES
            "tests/qiti_test_macros.hpp"
            "tests/test_qiti_FunctionCallData.cpp"
            "tests/test_qiti_FunctionData.cpp"
            "tests/test_qiti_Instrument.cpp"
            "tests/test_qiti_Profile.cpp"
            "tests/test_qiti_LeakSanitizer.cpp"
            "tests/test_qiti_ScopedNoHeapAllocations.cpp"
            "tests/test_qiti_ScopedQitiTest.cpp"
            "tests/test_qiti_TypeData.cpp"
            "tests/test_qiti_Utils.cpp"
        )
    else()
        # Other platforms: Full test set
        set(TEST_SOURCES
            "tests/qiti_test_macros.hpp"
            "tests/test_qiti_FunctionCallData.cpp"
            "tests/test_qiti_FunctionData.cpp"
            "tests/test_qiti_HotspotDetector.cpp"
            "tests/test_qiti_Instrument.cpp"
            "tests/test_qiti_Profile.cpp"
            "tests/test_qiti_LeakSanitizer.cpp"
            "tests/test_qiti_LockData.cpp"
            "tests/test_qiti_ScopedNoHeapAllocations.cpp"
            "tests/test_qiti_ScopedQitiTest.cpp"
            "tests/test_qiti_ThreadSanitizer.cpp"
            "tests/test_qiti_TypeData.cpp"
            "tests/test_qiti_Utils.cpp"
        )
    endif()

    # Function to configure common test settings
    function(configure_test_target target_name framework_libraries framework_definitions)
        if(APPLE)
            set_target_properties(${target_name} PROPERTIES
                XCODE_GENERATE_SCHEME ON
            )
        endif()

        target_link_libraries(${target_name} 
        PRIVATE
            qiti_example_target
            qiti_lib
            qiti_tests_client
            ${framework_libraries}
        )

        if(UNIX AND NOT APPLE)
            # On Linux, add "-rdynamic" so that all global symbols are exported into the dynamic symbol table.
            # Without this, functions defined in qiti_tests_client (and linked libraries) may not be visible to dladdr(),
            # causing dli_sname to be nullptr and leading to crashes when trying to copy the symbol name.
            target_link_options(${target_name} PRIVATE "-rdynamic")
        endif()

        target_compile_options(${target_name} 
        PRIVATE
            "-Werror"
            "-Wpedantic"
        )

        # ThreadSanitizer compile definition for test files
        if(QITI_ENABLE_THREAD_SANITIZER)
            target_compile_definitions(${target_name} PRIVATE QITI_ENABLE_THREAD_SANITIZER=1)
        endif()

        # Add framework-specific definitions
        if(framework_definitions)
            target_compile_definitions(${target_name} PRIVATE ${framework_definitions})
        endif()

        target_include_directories(${target_name} 
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/source
            ${CMAKE_CURRENT_SOURCE_DIR}/example/source
        )
        
        # On Windows, ensure DLLs and executables are in the same directory
        if(WIN32)
            set_target_properties(${target_name} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
            )
        endif()
    endfunction()

    # Catch2 Test Target
    add_executable(qiti_tests_catch2 ${TEST_SOURCES})
    configure_test_target(qiti_tests_catch2 "Catch2::Catch2WithMain" "")

    # GTest Test Target  
    add_executable(qiti_tests_gtest ${TEST_SOURCES})
    configure_test_target(qiti_tests_gtest "gtest_main" "QITI_USE_GTEST")

    # Default qiti_tests target (Catch2 for backward compatibility)
    add_executable(qiti_tests ${TEST_SOURCES})
    configure_test_target(qiti_tests "Catch2::Catch2WithMain" "")

    # ThreadSanitizer compile definition for qiti_tests_client (shared by all test targets)
    if(QITI_ENABLE_THREAD_SANITIZER)
        target_compile_definitions(qiti_tests_client PRIVATE QITI_ENABLE_THREAD_SANITIZER=1)
    endif()

    # Linux-specific linker options for qiti_tests_client (shared by all test targets)
    if(UNIX AND NOT APPLE)
        # On Linux, add "-rdynamic" so that all global symbols are exported into the dynamic symbol table.
        # Without this, functions defined in qiti_tests_client (and linked libraries) may not be visible to dladdr(),
        # causing dli_sname to be nullptr and leading to crashes when trying to copy the symbol name.
        target_link_options(qiti_tests_client INTERFACE "-rdynamic")
    endif()

    enable_testing()


    # Test discovery for Catch2 targets
    # Load and use the .cmake file provided by Catch2
    # https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md
    include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)

    # Test discovery for GTest targets
    include(GoogleTest)

    # When running ctest, you must add a configuration flag, e.g. "ctest -C Debug"
    catch_discover_tests(qiti_tests_catch2 DISCOVERY_MODE PRE_TEST)
    gtest_discover_tests(qiti_tests_gtest DISCOVERY_MODE PRE_TEST)
endif()

# =========================
#       Documentation
# =========================

if(PROJECT_IS_TOP_LEVEL)
    function(Doxygen input output)
        find_package(Doxygen)
        if (NOT DOXYGEN_FOUND)
            add_custom_target(doxygen COMMAND false
                COMMENT "Doxygen not found")
            return()
        endif()
        set(DOXYGEN_GENERATE_HTML YES)
        set(DOXYGEN_HTML_OUTPUT
            ${PROJECT_SOURCE_DIR}/${output}/html
        )

        set(DOXYGEN_FILE_PATTERNS "*.h" "*.hpp") # do not process .cpp files

        set(DOXYGEN_PREDEFINED ${COMPILE_DEFINITIONS})
        set(DOXYGEN_EXTRACT_ALL YES)
        set(DOXYGEN_INTERNAL_DOCS NO)
        set(DOXYGEN_EXTRACT_PRIVATE NO)
        set(DOXYGEN_EXTRACT_STATIC NO)
        set(DOXYGEN_EXTRACT_MACROS = YES)
        set(DOXYGEN_HIDE_UNDOC_CLASSES YES)
        set(DOXYGEN_HIDE_UNDOC_MEMBERS YES)
        set(DOXYGEN_SHOW_FILES NO)
        set(DOXYGEN_SHOW_NAMESPACES NO)
        set(DOXYGEN_VERBATIM_HEADERS NO)
        set(DOXYGEN_RECURSIVE YES)
        set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${PROJECT_SOURCE_DIR}/README.md)
        set(DOXYGEN_CALL_GRAPH YES)
        set(DOXYGEN_CALLER_GRAPH YES)
        set(DOXYGEN_DOT_GRAPH_MAX_NODES 100)
        set(DOXYGEN_SHOW_ENUM_VALUES YES)
        set(DOXYGEN_INCLUDE_PATH ${INCLUDE_DIRECTORIES})

        # completely mute warnings:
        set(DOXYGEN_QUIET              YES)

        # specifically turn off “doc-error” warnings (mismatches between your comments
        # and the actual code)
        set(DOXYGEN_WARN_IF_DOC_ERROR  NO)

        # if you’ve got stray \param or other tags pointing at non-existent symbols
        # you can silence those too
        set(DOXYGEN_WARN_NO_PARAMDOC   NO)

        # turn on preprocessing & macro-expansion
        set(DOXYGEN_ENABLE_PREPROCESSING YES)
        set(DOXYGEN_MACRO_EXPANSION    YES)
        set(DOXYGEN_EXPAND_ONLY_PREDEF YES)

        # force QITI_API* to be “nothing”
        list(APPEND DOXYGEN_PREDEFINED
            "QITI_API="
            "QITI_API_INTERNAL=")

        # if your Doxygen is new enough, also skip them entirely
        set(DOXYGEN_SKIP_FUNCTION_MACROS YES)

        # ensure the HTML output folder exists
        file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/${output}/html)

        doxygen_add_docs(doxygen
            ${PROJECT_SOURCE_DIR}/${input} ${PROJECT_SOURCE_DIR}/README.md 
            COMMENT "Generate HTML documentation"
        )

        set_target_properties(doxygen PROPERTIES
            XCODE_GENERATE_SCHEME ON
        )
    endfunction()

    Doxygen(source docs)
endif()