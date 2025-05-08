
#include <qiti_include.hpp>

#include "qiti_test_macros.hpp"

#include <catch2/catch_test_macros.hpp>

/** NOT static to purposely allow external linkage and visibility to QITI */
__attribute__((noinline)) __attribute__((optnone))
void testFuncProfile() noexcept
{
    volatile int _ = 42;
}

TEST_CASE("qiti::profile::resetProfiling()")
{
    qiti::resetAll();
    
    qiti::profile::beginProfilingFunction<&testFuncProfile>();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&testFuncProfile>());
    
    qiti::profile::resetProfiling();
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&testFuncProfile>());
    
    qiti::profile::beginProfilingAllFunctions();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&testFuncProfile>());
    
    qiti::profile::resetProfiling();
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&testFuncProfile>());
    
    qiti::resetAll();
}

TEST_CASE("qiti::profile::beginProfilingFunction()")
{
    qiti::resetAll();
    
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&testFuncProfile>());
    qiti::profile::beginProfilingFunction<&testFuncProfile>();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&testFuncProfile>());
    
    qiti::resetAll();
}

TEST_CASE("qiti::profile::endProfilingFunction()")
{
    qiti::resetAll();
    
    qiti::profile::beginProfilingFunction<&testFuncProfile>();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&testFuncProfile>());
    
    qiti::profile::endProfilingFunction<&testFuncProfile>();
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&testFuncProfile>());
    
    qiti::resetAll();
}

TEST_CASE("qiti::profile::beginProfilingAllFunctions()")
{
    qiti::resetAll();
    
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&testFuncProfile>());
    qiti::profile::beginProfilingAllFunctions();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&testFuncProfile>());
    
    qiti::resetAll();
}

TEST_CASE("qiti::profile::endProfilingAllFunctions()")
{
    qiti::resetAll();
    
    qiti::profile::beginProfilingAllFunctions();
    QITI_REQUIRE(qiti::profile::isProfilingFunction<&testFuncProfile>());
    
    qiti::profile::endProfilingAllFunctions();
    QITI_REQUIRE_FALSE(qiti::profile::isProfilingFunction<&testFuncProfile>());
    
    qiti::resetAll();
}

TEST_CASE("qiti::profile::isProfilingFunction()")
{
    // TODO: implement
}

TEST_CASE("qiti::profile::getNumHeapAllocationsOnCurrentThread()")
{
    qiti::resetAll();
    
    // TODO: implement (when more internals are guaranteed not to heap allocate)
    
    qiti::resetAll();
}
