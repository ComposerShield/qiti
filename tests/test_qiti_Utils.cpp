
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

QITI_TEST_CASE("qiti::getFunctionName()")
{
    qiti::ScopedQitiTest test;
    
    QITI_SECTION("Simple global namespace function from this translation unit")
    {
        std::string name = qiti::Profile::getFunctionName<&localTestFunc>();
        QITI_REQUIRE(name == "localTestFunc");
    }
    
    QITI_SECTION("Simple global namespace static function from this translation unit")
    {
        std::string name = qiti::Profile::getFunctionName<&localStaticTestFunc>();
        QITI_REQUIRE(name == "localStaticTestFunc");
    }
    
    QITI_SECTION("Simple global namespace inline function from this translation unit")
    {
        std::string name = qiti::Profile::getFunctionName<&localInlineTestFunc>();
        QITI_REQUIRE(name == "localInlineTestFunc");
    }
    
    QITI_SECTION("Multi-namespaced function")
    {
        std::string name = qiti::Profile::getFunctionName<&testFunc0>();
        QITI_REQUIRE(name == "qiti::example::utils::testFunc0");
    }
    
    QITI_SECTION("Complex, namespaced, templated/typedef, STL function")
    {
        std::string name = qiti::Profile::getFunctionName<&std::string::empty>();
        QITI_REQUIRE(name == "std::basic_string<char>::empty");
    }
}
