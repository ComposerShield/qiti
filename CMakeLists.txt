cmake_minimum_required(VERSION 3.14)

set(QITI_VERSION "0.0.1")
project(Qiti 
    VERSION ${QITI_VERSION} 
    LANGUAGES C CXX
)

if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"))
    message(FATAL_ERROR "Only Clang or Apple Clang compilers are supported")
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
add_library(qiti_tests_client OBJECT "source/client/qiti_tests_client.cpp")

if(PROJECT_IS_TOP_LEVEL)
    # Internal example app used for testing
    add_library(qiti_example_target SHARED ${EXAMPLE_SOURCES})

    target_link_libraries(qiti_example_target
    PRIVATE
        qiti_lib
    )
endif()

if(APPLE)
    set_target_properties(qiti_lib PROPERTIES
        VERSION     ${PROJECT_VERSION}
        OSX_ARCHITECTURES "${ARCHS}"
        XCODE_GENERATE_SCHEME ON
        INSTALL_NAME_DIR "@executable_path" # place next to executable within bundle
    )
    
    if(PROJECT_IS_TOP_LEVEL)
        set_target_properties(qiti_example_target PROPERTIES
            VERSION     ${PROJECT_VERSION}
            OSX_ARCHITECTURES "${ARCHS}"
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
    # Prevent inlining which breaks our ability to track memory
    "-fno-inline"
    # Enable Thread Sanitizer
    "-fsanitize=thread"
    # Generate debug info
    "-g" 
    # Disable Optimizations 
    "-O0"
)

target_compile_definitions(qiti_lib
PRIVATE
    QITI_DYLIB=1
PUBLIC
    QITI_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    QITI_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    QITI_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    QITI_VERSION=\"${PROJECT_VERSION}\" # need the escape-quoted string for a valid C literal:
)

target_link_options(qiti_lib
INTERFACE
    "-fsanitize=thread"
)

# =========================
#       Unit Testing
# =========================

if(PROJECT_IS_TOP_LEVEL)
    include(FetchContent)
        FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.5.0
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(qiti_tests
        "tests/qiti_test_macros.hpp"
        "tests/test_qiti_FunctionCallData.cpp"
        "tests/test_qiti_FunctionData.cpp"
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

    if(APPLE)
        set_target_properties(qiti_tests PROPERTIES
            OSX_ARCHITECTURES "${ARCHS}"
            XCODE_GENERATE_SCHEME ON
        )
    endif()

    target_link_libraries(qiti_tests 
    PRIVATE
        qiti_example_target
        qiti_lib
        qiti_tests_client
        Catch2::Catch2WithMain
    )

    if(UNIX AND NOT APPLE)
        # On Linux, add “-rdynamic” so that all global symbols are exported into the dynamic symbol table.
        # Without this, functions defined in qiti_tests_client (and linked libraries) may not be visible to dladdr(),
        # causing dli_sname to be nullptr and leading to crashes when trying to copy the symbol name.
        target_link_options(qiti_tests_client INTERFACE "-rdynamic")
    endif()

    target_compile_options(qiti_tests 
    PRIVATE
        "-Werror"
        "-Wpedantic"
    )

    enable_testing()

    if(APPLE)
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
    endif()

    target_include_directories(qiti_tests 
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
        ${CMAKE_CURRENT_SOURCE_DIR}/example/source
    )

    # Load and use the .cmake file provided by Catch2
    # https://github.com/catchorg/Catch2/blob/devel/docs/cmake-integration.md
    # We have to manually provide the source directory here for now

    include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)

    # When running ctest, you must add a configuration flag, e.g. "ctest -C Debug"
    catch_discover_tests(qiti_tests DISCOVERY_MODE PRE_TEST)
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