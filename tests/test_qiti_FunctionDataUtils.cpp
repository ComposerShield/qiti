
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

// Qiti Private API - not included in qiti_include.hpp
#include "qiti_FunctionDataUtils.hpp"

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

QITI_TEST_CASE("qiti::getFunctionName()", GetFunctionName)
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

//--------------------------------------------------------------------------
// Test access helper for private FunctionDataUtils methods

namespace qiti
{
struct FunctionDataUtilsTestAccess
{
    static const FunctionData* getFunctionData(const char* name) noexcept
    {
        return FunctionDataUtils::getFunctionData(name);
    }
};
} // namespace qiti

//--------------------------------------------------------------------------

QITI_TEST_CASE("qiti::FunctionDataUtils::getFunctionData(const char*)", GetFunctionDataByName)
{
    qiti::ScopedQitiTest test;

    QITI_SECTION("Returns FunctionData pointer when function is found by demangled name")
    {
        // First, profile localTestFunc so it gets registered in the function map
        qiti::Profile::beginProfilingFunction<&localTestFunc>();
        auto* expected = qiti::FunctionDataUtils::getFunctionData<&localTestFunc>();

        // Call it so it gets recorded
        localTestFunc();

        // Now look it up by its demangled name
        const qiti::FunctionData* result = qiti::FunctionDataUtilsTestAccess::getFunctionData("localTestFunc");

        QITI_REQUIRE(result != nullptr);
        QITI_REQUIRE(std::string(result->getFunctionName()) == "localTestFunc");
        QITI_REQUIRE(result == expected);
    }

    QITI_SECTION("Returns nullptr when function name is not found")
    {
        const qiti::FunctionData* result = qiti::FunctionDataUtilsTestAccess::getFunctionData("nonExistentFunction_xyz_12345");

        QITI_REQUIRE(result == nullptr);
    }

    QITI_SECTION("Returns FunctionData pointer for namespaced function")
    {
        // Profile a namespaced function
        qiti::Profile::beginProfilingFunction<&testFunc0>();
        auto* expected = qiti::FunctionDataUtils::getFunctionData<&testFunc0>();

        testFunc0();

        // Look it up by its fully qualified demangled name
        const qiti::FunctionData* result = qiti::FunctionDataUtilsTestAccess::getFunctionData("qiti::example::utils::testFunc0");

        QITI_REQUIRE(result != nullptr);
        QITI_REQUIRE(std::string(result->getFunctionName()) == "qiti::example::utils::testFunc0");
        QITI_REQUIRE(result == expected);
    }
}
