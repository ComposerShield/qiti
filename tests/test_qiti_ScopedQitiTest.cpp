
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

#include <string>

//--------------------------------------------------------------------------

QITI_TEST_CASE("qiti::ScopedQitiTest::getQitiVersionString()", ScopedQitiTestGetQitiVersionString)
{
    qiti::ScopedQitiTest test;

    // Brittle version tests, must be updated on each version update.
    QITI_CHECK("0.0.1" == std::string(qiti::ScopedQitiTest::getQitiVersionString()));
    QITI_CHECK(0       == qiti::ScopedQitiTest::getQitiVersionMajor());
    QITI_CHECK(0       == qiti::ScopedQitiTest::getQitiVersionMinor());
    QITI_CHECK(1       == qiti::ScopedQitiTest::getQitiVersionPatch());
}

QITI_TEST_CASE("qiti::ScopedQitiTest::enableProfilingOnAllFunctions()", ScopedQitiTestEnableProfilingOnAllFunctions)
{
    using qiti::example::profile::testFunc;
    {
        qiti::ScopedQitiTest test;
        
        QITI_REQUIRE_FALSE(qiti::Profile::isProfilingFunction<&testFunc>());
        test.enableProfilingOnAllFunctions(true);
        QITI_REQUIRE(qiti::Profile::isProfilingFunction<&testFunc>());
        test.enableProfilingOnAllFunctions(false);
        QITI_REQUIRE_FALSE(qiti::Profile::isProfilingFunction<&testFunc>());
        test.enableProfilingOnAllFunctions(true);
        QITI_REQUIRE(qiti::Profile::isProfilingFunction<&testFunc>());
    } // ScopedQitiTest destructs
    
    // should no longer profile after ScopedQitiTest destructs
    QITI_REQUIRE_FALSE(qiti::Profile::isProfilingFunction<&testFunc>());
}
