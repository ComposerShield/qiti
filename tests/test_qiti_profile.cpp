
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace qiti::example::profile;

TEST_CASE("qiti::profile::resetProfiling()")
{
    qiti::resetAll();
    
    qiti::profile::beginProfilingFunction<&testFunc>();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&testFunc>());
    
    qiti::profile::resetProfiling();
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&testFunc>());
    
    qiti::profile::beginProfilingAllFunctions();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&testFunc>());
    
    qiti::profile::resetProfiling();
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&testFunc>());
    
    qiti::resetAll();
}

TEST_CASE("qiti::profile::beginProfilingFunction()")
{
    qiti::resetAll();
    
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&testFunc>());
    qiti::profile::beginProfilingFunction<&testFunc>();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&testFunc>());
    
    qiti::resetAll();
}

TEST_CASE("qiti::profile::endProfilingFunction()")
{
    qiti::resetAll();
    
    qiti::profile::beginProfilingFunction<&testFunc>();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&testFunc>());
    
    qiti::profile::endProfilingFunction<&testFunc>();
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&testFunc>());
    
    qiti::resetAll();
}

TEST_CASE("qiti::profile::beginProfilingAllFunctions()")
{
    qiti::resetAll();
    
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&testFunc>());
    qiti::profile::beginProfilingAllFunctions();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&testFunc>());
    
    qiti::resetAll();
}

TEST_CASE("qiti::profile::endProfilingAllFunctions()")
{
    qiti::resetAll();
    
    qiti::profile::beginProfilingAllFunctions();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&testFunc>());
    
    qiti::profile::endProfilingAllFunctions();
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&testFunc>());
    
    qiti::resetAll();
}

TEST_CASE("qiti::profile::isProfilingFunction()")
{
    // TODO: implement
}

TEST_CASE("qiti::profile::begin/endProfilingType()")
{
    qiti::profile::beginProfilingType<ProfileTestType>();
    
    qiti::profile::endProfilingType<ProfileTestType>();
    // TODO: implement
}

TEST_CASE("qiti::profile::getNumHeapAllocationsOnCurrentThread()")
{
    qiti::resetAll();
    
    // TODO: implement (when more internals are guaranteed not to heap allocate)
    
    qiti::resetAll();
}
