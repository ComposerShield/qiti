
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"
// Basic Catch2 macros
#include <catch2/catch_test_macros.hpp>

#include <string>

//--------------------------------------------------------------------------

TEST_CASE("qiti::ScopedQitiTest::getQitiVersionString()")
{
    qiti::ScopedQitiTest test;

    // Brittle version tests, must be updated on each version update.
    CHECK("0.0.1" == std::string(qiti::ScopedQitiTest::getQitiVersionString()));
    CHECK(0       == qiti::ScopedQitiTest::getQitiVersionMajor());
    CHECK(0       == qiti::ScopedQitiTest::getQitiVersionMinor());
    CHECK(1       == qiti::ScopedQitiTest::getQitiVersionPatch());
}
