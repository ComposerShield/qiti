
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"
// Basic Catch2 macros
#include <catch2/catch_test_macros.hpp>

// Qiti Private API - not included in qiti_include.hpp
#include "qiti_utils.hpp"

#include <string>

/** NOT static to purposely allow external linkage and visibility to QITI */
__attribute__((noinline)) __attribute__((optnone))
[[nodiscard]] std::string demangleFunc(const char* mangled)
{
    char demangled[256] = {};
    qiti::demangle(mangled, demangled, sizeof(demangled));
    return std::string(demangled);
}

TEST_CASE("qiti::getFunctionName()")
{
    qiti::resetAll();
    
    SECTION("Simple static function from this translation unit")
    {
        auto name = qiti::getFunctionName<&demangleFunc>();
        QITI_REQUIRE(name == "demangleFunc");
    }
    
    SECTION("Complex, namespaced, templated/typedef, STL function")
    {
        auto name = qiti::getFunctionName<&std::string::empty>();
        QITI_REQUIRE(name == "std::basic_string<char>::empty");
    }
}

TEST_CASE("qiti::demangle() on valid Itanium‐ABI mangled names")
{
    SECTION("simple function with one int parameter")
    {
        QITI_REQUIRE( demangleFunc("_Z3fooi") == "foo(int)" );
    }

    SECTION("function with no parameters")
    {
        // _Z3barv → bar()
        QITI_REQUIRE( demangleFunc("_Z3barv") == "bar()" );
    }

    SECTION("namespaced function")
    {
        // _ZN2ns3bazEv → ns::baz()
        QITI_REQUIRE( demangleFunc("_ZN2ns3bazEv") == "ns::baz()" );
    }

    SECTION("class method")
    {
        // _ZN3Cls3quxEi → Cls::qux(int)
        QITI_REQUIRE( demangleFunc("_ZN3Cls3quxEi") == "Cls::qux(int)" );
    }

    SECTION("template instantiation")
    {
        // _ZN2ns3fooIiEET_S0_ → ns::foo<int>(int)
        QITI_REQUIRE( demangleFunc("_ZN2ns3fooIiEET_S0_") == "int ns::foo<int>(ns::foo)" );
    }
}

TEST_CASE("qiti::demangle() falls back on non-mangled input")
{
    SECTION("plain identifier")
    {
        const char* name = "main";
        QITI_REQUIRE( demangleFunc(name) == "main" );
    }

    SECTION("random string")
    {
        const char* bogus = "_not_a_real_mangled_name_";
        QITI_REQUIRE( demangleFunc(bogus) == bogus );
    }

    SECTION("empty string")
    {
        const char* empty = "";
        QITI_REQUIRE( demangleFunc(empty) == "" );
    }
}

TEST_CASE("qiti::getAddressForMangledFunctionName()")
{
    qiti::resetAll();
    
    auto* funcAddress = (void*)&demangleFunc;
    
    qiti::profile::beginProfilingAllFunctions();
    
    // Call function to create FunctionData
    (void)demangleFunc(""); // TODO: make it so we do not have to call this
    
    auto functionData = qiti::getFunctionData<&demangleFunc>();
    QITI_REQUIRE( functionData != nullptr );
    
    auto mangledName = functionData->getMangledFunctionName();
    auto* address = qiti::getAddressForMangledFunctionName(mangledName);
    
    QITI_REQUIRE( address == funcAddress );
    
    qiti::profile::endProfilingAllFunctions();
}
