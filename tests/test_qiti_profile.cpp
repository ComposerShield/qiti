
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("qiti::profile::resetProfiling()")
{
    qiti::resetAll();
    
    qiti::profile::beginProfilingFunction<&qiti::example::testFuncProfile>();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&qiti::example::testFuncProfile>());
    
    qiti::profile::resetProfiling();
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&qiti::example::testFuncProfile>());
    
    qiti::profile::beginProfilingAllFunctions();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&qiti::example::testFuncProfile>());
    
    qiti::profile::resetProfiling();
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&qiti::example::testFuncProfile>());
    
    qiti::resetAll();
}

TEST_CASE("qiti::profile::beginProfilingFunction()")
{
    qiti::resetAll();
    
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&qiti::example::testFuncProfile>());
    qiti::profile::beginProfilingFunction<&qiti::example::testFuncProfile>();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&qiti::example::testFuncProfile>());
    
    qiti::resetAll();
}

TEST_CASE("qiti::profile::endProfilingFunction()")
{
    qiti::resetAll();
    
    qiti::profile::beginProfilingFunction<&qiti::example::testFuncProfile>();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&qiti::example::testFuncProfile>());
    
    qiti::profile::endProfilingFunction<&qiti::example::testFuncProfile>();
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&qiti::example::testFuncProfile>());
    
    qiti::resetAll();
}

TEST_CASE("qiti::profile::beginProfilingAllFunctions()")
{
    qiti::resetAll();
    
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&qiti::example::testFuncProfile>());
    qiti::profile::beginProfilingAllFunctions();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&qiti::example::testFuncProfile>());
    
    qiti::resetAll();
}

TEST_CASE("qiti::profile::endProfilingAllFunctions()")
{
    qiti::resetAll();
    
    qiti::profile::beginProfilingAllFunctions();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&qiti::example::testFuncProfile>());
    
    qiti::profile::endProfilingAllFunctions();
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&qiti::example::testFuncProfile>());
    
    qiti::resetAll();
}

TEST_CASE("qiti::profile::isProfilingFunction()")
{
    // TODO: implement
}

TEST_CASE("qiti::profile::begin/endProfilingType()")
{
    qiti::profile::beginProfilingType<qiti::example::ProfileTestType>();
    
    qiti::profile::endProfilingType<qiti::example::ProfileTestType>();
    // TODO: implement
}

TEST_CASE("qiti::profile::getNumHeapAllocationsOnCurrentThread()")
{
    qiti::resetAll();
    
    // TODO: implement (when more internals are guaranteed not to heap allocate)
    
    qiti::resetAll();
}
