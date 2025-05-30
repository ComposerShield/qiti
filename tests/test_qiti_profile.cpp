
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"
// Basic Catch2 macros
#include <catch2/catch_test_macros.hpp>

// Qiti Private API - not included in qiti_include.hpp
#include "qiti_profile.hpp"

using namespace qiti::example::profile;

TEST_CASE("qiti::profile::resetProfiling()")
{
    qiti::ScopedQitiTest test;
    
    qiti::profile::beginProfilingFunction<&testFunc>();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&testFunc>());
    
    qiti::profile::resetProfiling();
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&testFunc>());
}

TEST_CASE("qiti::profile::beginProfilingFunction()")
{
    qiti::ScopedQitiTest test;
    
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&testFunc>());
    qiti::profile::beginProfilingFunction<&testFunc>();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&testFunc>());
}

TEST_CASE("qiti::profile::endProfilingFunction()")
{
    qiti::ScopedQitiTest test;
    
    qiti::profile::beginProfilingFunction<&testFunc>();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&testFunc>());
    
    qiti::profile::endProfilingFunction<&testFunc>();
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&testFunc>());
}

#if defined(__APPLE__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated"
TEST_CASE("qiti::profile::beginProfilingAllFunctions()")
{
    qiti::ScopedQitiTest test;
    
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&testFunc>());
    qiti::profile::beginProfilingAllFunctions();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&testFunc>());
    qiti::profile::endProfilingAllFunctions();
}

TEST_CASE("qiti::profile::endProfilingAllFunctions()")
{
    qiti::ScopedQitiTest test;
    
    qiti::profile::beginProfilingAllFunctions();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&testFunc>());
    
    qiti::profile::endProfilingAllFunctions();
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&testFunc>());
}
#pragma clang diagnostic pop
#endif // defined(__APPLE__)

TEST_CASE("qiti::profile::isProfilingFunction()")
{
    qiti::ScopedQitiTest test;
    
    // TODO: implement
}

TEST_CASE("qiti::profile::begin/endProfilingType()")
{
    qiti::ScopedQitiTest test;
    
    qiti::profile::beginProfilingType<ProfileTestType>();
    
    qiti::profile::endProfilingType<ProfileTestType>();
    // TODO: implement
}

TEST_CASE("qiti::profile::getNumHeapAllocationsOnCurrentThread()")
{
    qiti::ScopedQitiTest test;
    
    // TODO: implement (when more internals are guaranteed not to heap allocate)
}



