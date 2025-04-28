
#include "qiti_utils.hpp"
#include "qiti_test_macros.hpp"

#include <catch2/catch_test_macros.hpp>

[[nodiscard]] inline static std::string demangleFunc(const char* mangled)
{
    char demangled[256] = {};
    qiti::demangle(mangled, demangled, sizeof(demangled));
    return std::string(demangled);
}

TEST_CASE("qiti::demangle() on valid Itanium‐ABI mangled names", "[qiti::demangle]")
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
    
    qiti::shutdown();
}

TEST_CASE("qiti::demangle() falls back on non-mangled input", "[qiti::demangle]")
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
    
    qiti::shutdown();
}
