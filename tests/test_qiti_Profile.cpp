
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"
// Basic Catch2 macros
#include <catch2/catch_test_macros.hpp>

// Qiti Private API - not included in qiti_include.hpp
#include "qiti_Profile.hpp"

//--------------------------------------------------------------------------

using namespace qiti::example::profile;

//--------------------------------------------------------------------------

TEST_CASE("qiti::Profile::resetProfiling()")
{
    qiti::ScopedQitiTest test;
    
    qiti::Profile::beginProfilingFunction<&testFunc>();
    QITI_REQUIRE(qiti::Profile::isProfilingFunction<&testFunc>());
    
    qiti::Profile::resetProfiling();
    QITI_REQUIRE_FALSE(qiti::Profile::isProfilingFunction<&testFunc>());
}

TEST_CASE("qiti::Profile::{begin/end}ProfilingFunction() on free function")
{
    qiti::ScopedQitiTest test;
    
    QITI_REQUIRE_FALSE(qiti::Profile::isProfilingFunction<&testFunc>());
    qiti::Profile::beginProfilingFunction<&testFunc>();
    QITI_REQUIRE(qiti::Profile::isProfilingFunction<&testFunc>());
    
    qiti::Profile::endProfilingFunction<&testFunc>();
    QITI_REQUIRE_FALSE(qiti::Profile::isProfilingFunction<&testFunc>());
}

TEST_CASE("qiti::Profile::{begin/end}ProfilingFunction() on member function")
{
    qiti::ScopedQitiTest test;
    
    QITI_REQUIRE_FALSE(qiti::Profile::isProfilingFunction<&TestType::testFunc>());
    qiti::Profile::beginProfilingFunction<&TestType::testFunc>();
    QITI_REQUIRE(qiti::Profile::isProfilingFunction<&TestType::testFunc>());
    
    qiti::Profile::endProfilingFunction<&TestType::testFunc>();
    QITI_REQUIRE_FALSE(qiti::Profile::isProfilingFunction<&TestType::testFunc>());
}

#if defined(__APPLE__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated"
TEST_CASE("qiti::Profile::beginProfilingAllFunctions()")
{
    qiti::ScopedQitiTest test;
    
    QITI_REQUIRE_FALSE(qiti::Profile::isProfilingFunction<&testFunc>());
    qiti::Profile::beginProfilingAllFunctions();
    QITI_REQUIRE(qiti::Profile::isProfilingFunction<&testFunc>());
    qiti::Profile::endProfilingAllFunctions();
}

TEST_CASE("qiti::Profile::endProfilingAllFunctions()")
{
    qiti::ScopedQitiTest test;
    
    qiti::Profile::beginProfilingAllFunctions();
    QITI_REQUIRE(qiti::Profile::isProfilingFunction<&testFunc>());
    
    qiti::Profile::endProfilingAllFunctions();
    QITI_REQUIRE_FALSE(qiti::Profile::isProfilingFunction<&testFunc>());
}
#pragma clang diagnostic pop
#endif // defined(__APPLE__)

TEST_CASE("qiti::Profile::isProfilingFunction()")
{
    qiti::ScopedQitiTest test;
    
    // TODO: implement
}

TEST_CASE("qiti::Profile::begin/endProfilingType()")
{
    qiti::ScopedQitiTest test;
    
    qiti::Profile::beginProfilingType<TestType>();
    
    qiti::Profile::endProfilingType<TestType>();
    // TODO: implement
}

TEST_CASE("qiti::Profile::getNumHeapAllocationsOnCurrentThread()")
{
    qiti::ScopedQitiTest test;
    
    // Should have no heap allocs yet
    const auto numAllocsAtStartup = qiti::Profile::getNumHeapAllocationsOnCurrentThread();
    QITI_REQUIRE(numAllocsAtStartup == 0);
    
    // 1 heap allocation
    testHeapAllocation();
    const auto numAllocsAfterCallingTestFunc = qiti::Profile::getNumHeapAllocationsOnCurrentThread();
    QITI_CHECK(numAllocsAfterCallingTestFunc == 1);
    
    // Another heap allocation
    testHeapAllocation();
    const auto numAllocsAfterCallingTestFuncAgain = qiti::Profile::getNumHeapAllocationsOnCurrentThread();
    QITI_CHECK(numAllocsAfterCallingTestFuncAgain == 2);
}

#if defined(__APPLE__)
TEST_CASE("qiti::Profile::getNumHeapAllocationsOnCurrentThread() passing into Catch2 SECTION")
{
    qiti::ScopedQitiTest test;
    
    // Should have no heap allocs yet
    const auto numAllocsAtStartup = qiti::Profile::getNumHeapAllocationsOnCurrentThread();
    QITI_REQUIRE(numAllocsAtStartup == 0);
    
    // When passing into a SECTION, Catch2 makes multiple heap allocations.
    // We want to ensure we ignore those and not treat them as part of what the user is testing.
    SECTION("Example Catch2 SECTION")
    {
        // Should still have no heap allocs
        const auto numAllocsAtStartOfSection = qiti::Profile::getNumHeapAllocationsOnCurrentThread();
        QITI_REQUIRE(numAllocsAtStartOfSection == 0);
    }
}
#endif // defined(__APPLE__)
