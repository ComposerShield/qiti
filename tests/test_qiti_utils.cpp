
#include "qiti_utils.hpp"
#include "qiti_test_macros.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("qiti::demangle() on valid Itanium‐ABI mangled names", "[qiti::demangle]")
{
    SECTION("simple function with one int parameter")
    {
        // _Z3fooi → foo(int)
        QITI_REQUIRE( qiti::demangle("_Z3fooi") == "foo(int)" );
    }

    SECTION("function with no parameters")
    {
        // _Z3barv → bar()
        QITI_REQUIRE( qiti::demangle("_Z3barv") == "bar()" );
    }

    SECTION("namespaced function")
    {
        // _ZN2ns3bazEv → ns::baz()
        QITI_REQUIRE( qiti::demangle("_ZN2ns3bazEv") == "ns::baz()" );
    }

    SECTION("class method")
    {
        // _ZN3Cls3quxEi → Cls::qux(int)
        QITI_REQUIRE( qiti::demangle("_ZN3Cls3quxEi") == "Cls::qux(int)" );
    }

    SECTION("template instantiation")
    {
        // _ZN2ns3fooIiEET_S0_ → ns::foo<int>(int)
        QITI_REQUIRE( qiti::demangle("_ZN2ns3fooIiEET_S0_") == "int ns::foo<int>(ns::foo)" );
    }
}

TEST_CASE("qiti::demangle() falls back on non-mangled input", "[qiti::demangle]")
{
    SECTION("plain identifier")
    {
        const char* name = "main";
        QITI_REQUIRE( qiti::demangle(name) == "main" );
    }

    SECTION("random string")
    {
        const char* bogus = "_not_a_real_mangled_name_";
        QITI_REQUIRE( qiti::demangle(bogus) == bogus );
    }

    SECTION("empty string")
    {
        const char* empty = "";
        QITI_REQUIRE( qiti::demangle(empty) == "" );
    }
}
