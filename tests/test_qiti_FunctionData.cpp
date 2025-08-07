
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

#include <algorithm>
#include <random>
#include <thread>
#include <vector>

#include <iostream>

//--------------------------------------------------------------------------

/** NOT static to purposely allow external linkage and visibility to QITI */
__attribute__((noinline))
__attribute__((optnone))
void testFunc() noexcept
{
    volatile int _ = 42;
}

/** Test function with variable execution time for min/max testing */
__attribute__((noinline))
__attribute__((optnone))
void testFuncWithDelay(int delayMs) noexcept
{
    volatile int sum = 0;
    // Create variable execution time by doing different amounts of work
    for(int i = 0; i < delayMs * 1000; ++i) {
        sum += i;
    }
}

//--------------------------------------------------------------------------

QITI_TEST_CASE("qiti::FunctionData::getFunctionName()", FunctionDataGetFunctionName)
{
    qiti::ScopedQitiTest test;

    {
        qiti::Profile::beginProfilingFunction<&testFunc>();
        auto functionData = qiti::Utils::getFunctionData<&testFunc>();
        std::string name = functionData->getFunctionName();
        QITI_CHECK(name == "testFunc");
    }
}

QITI_TEST_CASE("qiti::FunctionData::getNumTimesCalled()", FunctionDataGetNumTimesCalled)
{
    qiti::ScopedQitiTest test;
    
    qiti::Profile::beginProfilingFunction<&testFunc>();
    
    auto funcData = qiti::Utils::getFunctionData<&testFunc>();
    QITI_REQUIRE(funcData != nullptr);
    
    QITI_SECTION("Called twice")
    {
        testFunc();
        testFunc();
        QITI_CHECK(funcData->getNumTimesCalled() == 2);
    }
    
    QITI_SECTION("Not called")
    {
        QITI_CHECK(funcData->getNumTimesCalled() == 0);
    }
    
    QITI_SECTION("Called twice on two different threads")
    {
        std::thread t([]{ testFunc(); });
        testFunc();
        t.join();
        QITI_CHECK(funcData->getNumTimesCalled() == 2);
    }
}

QITI_TEST_CASE("qiti::FunctionData::getNumTimesCalled(), using static constructor", FunctionDataGetNumTimesCalledStaticConstructor)
{
    qiti::ScopedQitiTest test;
    
    auto funcData = qiti::FunctionData::getFunctionData<&testFunc>();
    QITI_REQUIRE(funcData != nullptr);
    
    QITI_SECTION("Called once")
    {
        testFunc();
        QITI_CHECK(funcData->getNumTimesCalled() == 1);
    }
    
    QITI_SECTION("Not called")
    {
        QITI_CHECK(funcData->getNumTimesCalled() == 0);
    }
}

QITI_TEST_CASE("qiti::FunctionData::wasCalledOnThread()", FunctionDataWasCalledOnThread)
{
    qiti::ScopedQitiTest test;
    
    qiti::Profile::beginProfilingFunction<&testFunc>();
    
    auto funcData = qiti::Utils::getFunctionData<&testFunc>();
    QITI_REQUIRE(funcData != nullptr);
    
    QITI_SECTION("Function called on current thread")
    {
        testFunc();
        
        std::thread::id currentThread = std::this_thread::get_id();
        
        QITI_CHECK(funcData->wasCalledOnThread(currentThread));
    }
    
    QITI_SECTION("Function never called")
    {
        std::thread::id currentThread = std::this_thread::get_id();
        
        QITI_CHECK(! funcData->wasCalledOnThread(currentThread));
    }
    
    QITI_SECTION("Function called on custom thread")
    {
        std::thread thread([]{ testFunc(); });
        auto id = thread.get_id();
        thread.join(); // sync with thread to ensure function was called
        
        QITI_CHECK(funcData->wasCalledOnThread(id));
    }
}

QITI_TEST_CASE("qiti::FunctionData::getAllProfiledFunctionData()", FunctionDataGetAllProfiledFunctionData)
{
    qiti::ScopedQitiTest test;
    
    // Profile two functions
    qiti::Profile::beginProfilingFunction<&testFunc>();
    qiti::Profile::beginProfilingFunction<&qiti::example::FunctionCallData::testHeapAllocation>();
    
    // Run the two functions
    testFunc();
    qiti::example::FunctionCallData::testHeapAllocation();
    
    // getAllProfiledFunctionData() contains our two functions
    {
        auto numHeapAllocsBefore = qiti::Profile::getNumHeapAllocationsOnCurrentThread();
        auto allFunctions = qiti::FunctionData::getAllProfiledFunctionData();
        QITI_REQUIRE(allFunctions.size() >= 2);
        QITI_REQUIRE(numHeapAllocsBefore == qiti::Profile::getNumHeapAllocationsOnCurrentThread());
        
        auto containsFunc = [&allFunctions](const std::string& funcName)->bool
        {
            for (const auto* func : allFunctions)
                if (std::string(func->getFunctionName()) == funcName)
                    return true;
            return false;
        };
        
        QITI_CHECK(containsFunc("testFunc"));
        QITI_CHECK(containsFunc("qiti::example::FunctionCallData::testHeapAllocation"));
        QITI_REQUIRE_FALSE(containsFunc("randomFuncNameThatWeDidNotCall"));
    }
    
    // Reset
    test.reset(false);
    
    // No profiled functions should be available
    auto allFunctionsAfterReset = qiti::FunctionData::getAllProfiledFunctionData();
    QITI_REQUIRE(allFunctionsAfterReset.size() == 0);
}

QITI_TEST_CASE("qiti::FunctionData::getMinTimeSpentInFunctionCpu_ns()", FunctionDataGetMinTimeSpentInFunctionCpu)
{
    qiti::ScopedQitiTest test;
    
    auto funcData = qiti::FunctionData::getFunctionData<&testFuncWithDelay>();
    QITI_REQUIRE(funcData != nullptr);
    
    QITI_SECTION("No calls made - should return 0")
    {
        QITI_CHECK(funcData->getMinTimeSpentInFunctionCpu_ns() == 0);
    }
    
    QITI_SECTION("Single call")
    {
        testFuncWithDelay(1); // Small delay
        uint64_t minTime = funcData->getMinTimeSpentInFunctionCpu_ns();
        QITI_CHECK(minTime > 0); // Should have some measurable time
    }
    
    QITI_SECTION("Multiple calls with different execution times")
    {
        // Make calls with different execution times
        testFuncWithDelay(5);  // Longer delay
        testFuncWithDelay(1);  // Shorter delay  
        testFuncWithDelay(3);  // Medium delay
        
        uint64_t minTime = funcData->getMinTimeSpentInFunctionCpu_ns();
        uint64_t maxTime = funcData->getMaxTimeSpentInFunctionCpu_ns();
        
        QITI_CHECK(minTime > 0);
        QITI_CHECK(maxTime > 0);
        QITI_CHECK(minTime <= maxTime); // Min should be <= max
        QITI_CHECK(funcData->getNumTimesCalled() == 3);
    }
}

QITI_TEST_CASE("qiti::FunctionData::getMaxTimeSpentInFunctionCpu_ns()", FunctionDataGetMaxTimeSpentInFunctionCpu)
{
    qiti::ScopedQitiTest test;
    
    auto funcData = qiti::FunctionData::getFunctionData<&testFuncWithDelay>();
    QITI_REQUIRE(funcData != nullptr);
    
    QITI_SECTION("No calls made - should return 0")
    {
        QITI_CHECK(funcData->getMaxTimeSpentInFunctionCpu_ns() == 0);
    }
    
    QITI_SECTION("Single call")
    {
        testFuncWithDelay(1); // Small delay
        uint64_t maxTime = funcData->getMaxTimeSpentInFunctionCpu_ns();
        QITI_CHECK(maxTime > 0); // Should have some measurable time
    }
    
    QITI_SECTION("Multiple calls - max should track longest execution")
    {
        testFuncWithDelay(1);  // Short
        uint64_t timeAfterFirst = funcData->getMaxTimeSpentInFunctionCpu_ns();
        
        testFuncWithDelay(5);  // Longer - should become new max
        uint64_t timeAfterSecond = funcData->getMaxTimeSpentInFunctionCpu_ns();
        
        testFuncWithDelay(2);  // Medium - shouldn't change max
        uint64_t timeAfterThird = funcData->getMaxTimeSpentInFunctionCpu_ns();
        
        QITI_CHECK(timeAfterFirst > 0);
        QITI_CHECK(timeAfterSecond >= timeAfterFirst); // Second call was longer
        QITI_CHECK(timeAfterThird == timeAfterSecond); // Third call shouldn't change max
        QITI_CHECK(funcData->getNumTimesCalled() == 3);
    }
}

QITI_TEST_CASE("qiti::FunctionData::getMinTimeSpentInFunctionWallClock_ns()", FunctionDataGetMinTimeSpentInFunctionWallClock)
{
    qiti::ScopedQitiTest test;
    
    auto funcData = qiti::FunctionData::getFunctionData<&testFuncWithDelay>();
    QITI_REQUIRE(funcData != nullptr);
    
    QITI_SECTION("No calls made - should return 0")
    {
        QITI_CHECK(funcData->getMinTimeSpentInFunctionWallClock_ns() == 0);
    }
    
    QITI_SECTION("Multiple calls with different execution times")
    {
        testFuncWithDelay(3);  // Medium delay
        testFuncWithDelay(1);  // Shorter delay
        testFuncWithDelay(5);  // Longer delay
        
        uint64_t minTime = funcData->getMinTimeSpentInFunctionWallClock_ns();
        uint64_t maxTime = funcData->getMaxTimeSpentInFunctionWallClock_ns();
        
        QITI_CHECK(minTime > 0);
        QITI_CHECK(maxTime > 0);
        QITI_CHECK(minTime <= maxTime); // Min should be <= max
        QITI_CHECK(funcData->getNumTimesCalled() == 3);
    }
}

QITI_TEST_CASE("qiti::FunctionData::getMaxTimeSpentInFunctionWallClock_ns()", FunctionDataGetMaxTimeSpentInFunctionWallClock)
{
    qiti::ScopedQitiTest test;
    
    auto funcData = qiti::FunctionData::getFunctionData<&testFuncWithDelay>();
    QITI_REQUIRE(funcData != nullptr);
    
    QITI_SECTION("No calls made - should return 0")
    {
        QITI_CHECK(funcData->getMaxTimeSpentInFunctionWallClock_ns() == 0);
    }
    
    QITI_SECTION("Single call")
    {
        testFuncWithDelay(2);
        uint64_t maxTime = funcData->getMaxTimeSpentInFunctionWallClock_ns();
        QITI_CHECK(maxTime > 0); // Should have some measurable time
    }
    
    QITI_SECTION("CPU vs WallClock consistency check")
    {
        testFuncWithDelay(2);
        testFuncWithDelay(4);
        
        uint64_t minCpu = funcData->getMinTimeSpentInFunctionCpu_ns();
        uint64_t maxCpu = funcData->getMaxTimeSpentInFunctionCpu_ns();
        uint64_t minWall = funcData->getMinTimeSpentInFunctionWallClock_ns();
        uint64_t maxWall = funcData->getMaxTimeSpentInFunctionWallClock_ns();
        
        // Both timing methods should show that min <= max
        QITI_CHECK(minCpu <= maxCpu);
        QITI_CHECK(minWall <= maxWall);
        QITI_CHECK(minCpu > 0);
        QITI_CHECK(minWall > 0);
        QITI_CHECK(funcData->getNumTimesCalled() == 2);
    }
}
