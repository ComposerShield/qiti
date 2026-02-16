
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

QITI_TEST_CASE("qiti::ScopedQitiTest::getLengthOfTest_ns()", ScopedQitiTestGetLengthOfTestNs)
{
    qiti::ScopedQitiTest test;

    auto elapsed_ns = test.getLengthOfTest_ns();
    QITI_CHECK(elapsed_ns >= 0);
}

QITI_TEST_CASE("qiti::ScopedQitiTest::reset(true)", ScopedQitiTestResetWithStartTime)
{
    qiti::ScopedQitiTest test;

    // Call reset with resetTestStartTime = true to cover that code path
    test.reset(true);

    // After resetting the start time, the elapsed time should be near zero
    auto elapsed_ns = test.getLengthOfTest_ns();
    QITI_CHECK(elapsed_ns >= 0);
}

QITI_TEST_CASE("qiti::isThreadSanitizerEnabled()", IsThreadSanitizerEnabled)
{
    qiti::ScopedQitiTest test;

    bool enabled = qiti::isThreadSanitizerEnabled();

#ifdef QITI_ENABLE_CLANG_THREAD_SANITIZER
    QITI_REQUIRE(enabled);
#else
    QITI_REQUIRE_FALSE(enabled);
#endif
}
