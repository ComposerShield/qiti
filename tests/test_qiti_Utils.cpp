
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

// Qiti Private API - not included in qiti_include.hpp
#include "qiti_Utils.hpp"

#include <iostream>
#include <string>

#ifdef _WIN32
// Debug printing for Windows crash debugging
#include <cstdio>
#define DEBUG_PRINT(...) do { printf(__VA_ARGS__); fflush(stdout); } while(0)

// Static initialization debug - this will print during global constructor phase
static struct DebugInit {
    DebugInit() { 
        DEBUG_PRINT("DEBUG: Static initialization starting for test_qiti_Utils.cpp\n");
    }
} g_debugInit;

#else
#define DEBUG_PRINT(...) do { } while(0)
#endif

//--------------------------------------------------------------------------

using namespace qiti::example::utils;

__attribute__((optnone))
__attribute__((noinline))
void localTestFunc() noexcept
{
    volatile int _ = 42;
}

__attribute__((optnone))
__attribute__((noinline))
static void localStaticTestFunc() noexcept
{
    volatile int _ = 42;
}

inline void localInlineTestFunc() noexcept
{
    volatile int _ = 42;
}

//--------------------------------------------------------------------------

QITI_TEST_CASE("qiti::getFunctionName()", GetFunctionName)
{
    DEBUG_PRINT("DEBUG: Entering getFunctionName test\n");
    
    DEBUG_PRINT("DEBUG: About to create ScopedQitiTest\n");
    qiti::ScopedQitiTest test;
    DEBUG_PRINT("DEBUG: ScopedQitiTest created successfully\n");
    
    QITI_SECTION("Simple global namespace function from this translation unit")
    {
        DEBUG_PRINT("DEBUG: Entering simple function section\n");
        std::string name = qiti::Profile::getFunctionName<&localTestFunc>();
        DEBUG_PRINT("DEBUG: Got function name: %s\n", name.c_str());
        QITI_REQUIRE(name == "localTestFunc");
        DEBUG_PRINT("DEBUG: Simple function section completed\n");
    }
    
    QITI_SECTION("Simple global namespace static function from this translation unit")
    {
        DEBUG_PRINT("DEBUG: Entering static function section\n");
        std::string name = qiti::Profile::getFunctionName<&localStaticTestFunc>();
        DEBUG_PRINT("DEBUG: Got static function name: %s\n", name.c_str());
        QITI_REQUIRE(name == "localStaticTestFunc");
        DEBUG_PRINT("DEBUG: Static function section completed\n");
    }
    
    QITI_SECTION("Simple global namespace inline function from this translation unit")
    {
        DEBUG_PRINT("DEBUG: Entering inline function section\n");
        std::string name = qiti::Profile::getFunctionName<&localInlineTestFunc>();
        DEBUG_PRINT("DEBUG: Got inline function name: %s\n", name.c_str());
        QITI_REQUIRE(name == "localInlineTestFunc");
        DEBUG_PRINT("DEBUG: Inline function section completed\n");
    }
    
    QITI_SECTION("Multi-namespaced function")
    {
        DEBUG_PRINT("DEBUG: Entering multi-namespaced function section\n");
        std::string name = qiti::Profile::getFunctionName<&testFunc0>();
        DEBUG_PRINT("DEBUG: Got namespaced function name: %s\n", name.c_str());
        QITI_REQUIRE(name == "qiti::example::utils::testFunc0");
        DEBUG_PRINT("DEBUG: Multi-namespaced function section completed\n");
    }
    
    QITI_SECTION("Complex, namespaced, templated/typedef, STL function")
    {
        DEBUG_PRINT("DEBUG: Entering STL function section\n");
        std::string name = qiti::Profile::getFunctionName<&std::string::empty>();
        DEBUG_PRINT("DEBUG: Got STL function name: %s\n", name.c_str());
        QITI_REQUIRE(name == "std::basic_string<char>::empty");
        DEBUG_PRINT("DEBUG: STL function section completed\n");
    }
    
    DEBUG_PRINT("DEBUG: getFunctionName test completed successfully\n");
}
