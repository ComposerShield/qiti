
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

// Qiti Private API - not included in qiti_include.hpp
#include "qiti_Profile.hpp"

//--------------------------------------------------------------------------

using namespace qiti::example::profile;

//--------------------------------------------------------------------------

QITI_TEST(Profile, beginProfilingFunction)
{
    qiti::ScopedQitiTest test;
    
    qiti::Profile::beginProfilingFunction<&testFunc>();
    QITI_REQUIRE(qiti::Profile::isProfilingFunction<&testFunc>());
    
    qiti::Profile::resetProfiling();
    QITI_REQUIRE_FALSE(qiti::Profile::isProfilingFunction<&testFunc>());
}

QITI_TEST(Profile, begin_endProfilingFunction_on_free_function)
{
    qiti::ScopedQitiTest test;
    
    QITI_REQUIRE_FALSE(qiti::Profile::isProfilingFunction<&testFunc>());
    qiti::Profile::beginProfilingFunction<&testFunc>();
    QITI_REQUIRE(qiti::Profile::isProfilingFunction<&testFunc>());
    
    qiti::Profile::endProfilingFunction<&testFunc>();
    QITI_REQUIRE_FALSE(qiti::Profile::isProfilingFunction<&testFunc>());
}

QITI_TEST(Profile, begin_endProfilingFunction_on_member_function)
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
QITI_TEST(Profile, beginProfilingAllFunctions)
{
    qiti::ScopedQitiTest test;
    
    QITI_REQUIRE_FALSE(qiti::Profile::isProfilingFunction<&testFunc>());
    qiti::Profile::beginProfilingAllFunctions();
    QITI_REQUIRE(qiti::Profile::isProfilingFunction<&testFunc>());
    qiti::Profile::endProfilingAllFunctions();
}

QITI_TEST(Profile, endProfilingAllFunctions)
{
    qiti::ScopedQitiTest test;
    
    qiti::Profile::beginProfilingAllFunctions();
    QITI_REQUIRE(qiti::Profile::isProfilingFunction<&testFunc>());
    
    qiti::Profile::endProfilingAllFunctions();
    QITI_REQUIRE_FALSE(qiti::Profile::isProfilingFunction<&testFunc>());
}
#pragma clang diagnostic pop
#endif // defined(__APPLE__)

QITI_TEST(Profile, isProfilingFunction)
{
    qiti::ScopedQitiTest test;
    
    // TODO: implement
}

QITI_TEST(Profile, begin_endProfilingType)
{
    qiti::ScopedQitiTest test;
    
    qiti::Profile::beginProfilingType<TestType>();
    
    qiti::Profile::endProfilingType<TestType>();
    // TODO: implement
}

QITI_TEST(Profile, getNumHeapAllocationsOnCurrentThread)
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

QITI_TEST(Profile, getNumHeapAllocationsOnCurrentThread_passing_into_SECTION)
{
    qiti::ScopedQitiTest test;
    
    // Should have no heap allocs yet
    const auto numAllocsAtStartup = qiti::Profile::getNumHeapAllocationsOnCurrentThread();
    QITI_REQUIRE(numAllocsAtStartup == 0);
    
    // When passing into a SECTION, Catch2 makes multiple heap allocations.
    // We want to ensure we ignore those and not treat them as part of what the user is testing.
    QITI_SECTION("Example Catch2 SECTION")
    {
        // Should still have no heap allocs
        const auto numAllocsAtStartOfSection = qiti::Profile::getNumHeapAllocationsOnCurrentThread();
        QITI_REQUIRE(numAllocsAtStartOfSection == 0);
    }
}

