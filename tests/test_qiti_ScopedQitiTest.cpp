
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

#include <string>

//--------------------------------------------------------------------------

QITI_TEST_CASE("qiti::ScopedQitiTest::getQitiVersionString()")
{
    qiti::ScopedQitiTest test;

    // Brittle version tests, must be updated on each version update.
    QITI_CHECK("0.0.1" == std::string(qiti::ScopedQitiTest::getQitiVersionString()));
    QITI_CHECK(0       == qiti::ScopedQitiTest::getQitiVersionMajor());
    QITI_CHECK(0       == qiti::ScopedQitiTest::getQitiVersionMinor());
    QITI_CHECK(1       == qiti::ScopedQitiTest::getQitiVersionPatch());
}
