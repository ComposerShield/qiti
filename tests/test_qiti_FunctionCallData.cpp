
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

//--------------------------------------------------------------------------

using namespace qiti::example::FunctionCallData;

//--------------------------------------------------------------------------

QITI_TEST_CASE("qiti::FunctionCallData::getNumHeapAllocations()", FunctionCallDataGetNumHeapAllocations)
{
    qiti::ScopedQitiTest test;
    
    QITI_SECTION("1 heap allocations")
    {
        auto funcData = qiti::FunctionData::getFunctionData<&testHeapAllocation>();
        QITI_REQUIRE(funcData != nullptr);
        
        // Call twice
        testHeapAllocation();
        testHeapAllocation();
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        auto numHeapAllocations = lastFunctionCall.getNumHeapAllocations();
        QITI_REQUIRE(numHeapAllocations == 1);
    }
    
    QITI_SECTION("0 heap allocation")
    {
        auto funcData = qiti::FunctionData::getFunctionData<&testNoHeapAllocation>();
        QITI_REQUIRE(funcData != nullptr);
        
        testNoHeapAllocation();
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        QITI_REQUIRE(lastFunctionCall.getNumHeapAllocations() == 0);
    }
}

QITI_TEST_CASE("qiti::FunctionCallData::getAmountHeapAllocated()", FunctionCallDataGetAmountHeapAllocated)
{
    qiti::ScopedQitiTest test;
    
    QITI_SECTION("1 heap allocations")
    {
        auto funcData = qiti::FunctionData::getFunctionData<&testHeapAllocation>();
        QITI_REQUIRE(funcData != nullptr);
        
        testHeapAllocation(); // contains 1 int allocaation
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        auto amountAllocated = lastFunctionCall.getAmountHeapAllocated();
        QITI_REQUIRE(amountAllocated == sizeof(int));
    }
}

QITI_TEST_CASE("qiti::FunctionCallData::getThreadThatCalledFunction()", FunctionCallDataGetThreadThatCalledFunction)
{
    qiti::ScopedQitiTest test;
    
    auto funcData = qiti::FunctionData::getFunctionData<&testHeapAllocation>();
    QITI_REQUIRE(funcData != nullptr);
    
    const auto currentThreadID = std::this_thread::get_id();
    
    QITI_SECTION("Called from unit test thread")
    {
        testHeapAllocation();
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        auto threadThatCalledFunction = lastFunctionCall.getThreadThatCalledFunction();
        QITI_REQUIRE(threadThatCalledFunction == currentThreadID); // called from this thread
    }
    
    QITI_SECTION("Called from custom thread")
    {
        std::thread t([]
        {
            testHeapAllocation();
        });
        auto tThreadID = t.get_id();
        
        t.join();
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        auto threadThatCalledFunction = lastFunctionCall.getThreadThatCalledFunction();
        QITI_REQUIRE_FALSE(threadThatCalledFunction == currentThreadID);
        QITI_REQUIRE(threadThatCalledFunction == tThreadID); // called from t thread
    }
}

#ifndef _WIN32 // CPU Time feature not supported on Windows
QITI_TEST_CASE("qiti::FunctionCallData::getTimeSpentInFunction", FunctionCallDataGetTimeSpentInFunction)
{
    qiti::ScopedQitiTest test;
    
    QITI_SECTION("Clock time always greater than or equal to CPU time")
    {
        auto funcData = qiti::FunctionData::getFunctionData<&testHeapAllocation>();
        QITI_REQUIRE(funcData != nullptr);
        
        testHeapAllocation();
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        
        auto timeSpentCpu   = lastFunctionCall.getTimeSpentInFunctionCpu_ns();
        auto timeSpentClock = lastFunctionCall.getTimeSpentInFunctionWallClock_ns();
        QITI_REQUIRE(timeSpentClock >= timeSpentCpu);
    }
    
    QITI_SECTION("Slow work results in more time elapsed than fast work")
    {
        auto fastWorkFuncData = qiti::FunctionData::getFunctionData<&fastWork>();
        QITI_REQUIRE(fastWorkFuncData != nullptr);
        
        auto slowWorkFuncData = qiti::FunctionData::getFunctionData<&slowWork>();
        QITI_REQUIRE(slowWorkFuncData != nullptr);
        
        // Call once to "warm up" CPU caches, branch predictors, etc.
        // First calls can have unpredictable performance overhead
        (void)fastWork(); // "cold call"
        (void)slowWork(); // "cold call"
        
        // Calls to be tested
        (void)fastWork();
        (void)slowWork();
        
        auto fastWorkLastFunctionCall = fastWorkFuncData->getLastFunctionCall();
        auto slowWorkLastFunctionCall = slowWorkFuncData->getLastFunctionCall();
        
        auto fastWorkTimeSpentCpu   = fastWorkLastFunctionCall.getTimeSpentInFunctionCpu_ns();
        auto slowWorkTimeSpentCpu   = slowWorkLastFunctionCall.getTimeSpentInFunctionCpu_ns();
        
        auto fastWorkTimeSpentClock = fastWorkLastFunctionCall.getTimeSpentInFunctionWallClock_ns();
        auto slowWorkTimeSpentClock = slowWorkLastFunctionCall.getTimeSpentInFunctionWallClock_ns();
        
        QITI_REQUIRE(slowWorkTimeSpentCpu >= fastWorkTimeSpentCpu);
        QITI_REQUIRE(slowWorkTimeSpentClock >= fastWorkTimeSpentClock);
    }
}
#endif // ! _WIN32

QITI_TEST_CASE("qiti::FunctionCallData move constructor", FunctionCallDataMoveConstructor)
{
    qiti::ScopedQitiTest test;

    auto funcData = qiti::FunctionData::getFunctionData<&testHeapAllocation>();
    QITI_REQUIRE(funcData != nullptr);

    testHeapAllocation();

    auto original = funcData->getLastFunctionCall();
    auto numAllocsBefore = original.getNumHeapAllocations();
    auto threadBefore = original.getThreadThatCalledFunction();

    // Move construct
    qiti::FunctionCallData moved(std::move(original));

    QITI_REQUIRE(moved.getNumHeapAllocations() == numAllocsBefore);
    QITI_REQUIRE(moved.getThreadThatCalledFunction() == threadBefore);
}

QITI_TEST_CASE("qiti::FunctionCallData move assignment", FunctionCallDataMoveAssignment)
{
    qiti::ScopedQitiTest test;

    auto funcData = qiti::FunctionData::getFunctionData<&testHeapAllocation>();
    QITI_REQUIRE(funcData != nullptr);

    testHeapAllocation();

    auto original = funcData->getLastFunctionCall();
    auto numAllocsBefore = original.getNumHeapAllocations();
    auto threadBefore = original.getThreadThatCalledFunction();

    // Move assign into a copy-initialized target (avoids default constructor)
    auto moved = funcData->getLastFunctionCall();
    [[maybe_unused]] auto& moveResult = (moved = std::move(original));

    QITI_REQUIRE(moved.getNumHeapAllocations() == numAllocsBefore);
    QITI_REQUIRE(moved.getThreadThatCalledFunction() == threadBefore);
}

#ifndef _WIN32 // CPU Time feature not supported on Windows
QITI_TEST_CASE("qiti::FunctionCallData::getTimeSpentInFunctionCpu_ms()", FunctionCallDataGetTimeSpentCpuMs)
{
    qiti::ScopedQitiTest test;

    auto funcData = qiti::FunctionData::getFunctionData<&slowWork>();
    QITI_REQUIRE(funcData != nullptr);

    // Warm up
    (void)slowWork();

    // Call to be tested
    (void)slowWork();

    auto lastCall = funcData->getLastFunctionCall();

    auto cpuTimeMs = lastCall.getTimeSpentInFunctionCpu_ms();
    auto cpuTimeNs = lastCall.getTimeSpentInFunctionCpu_ns();

    // slowWork does 100,000 iterations, should take at least 1ms of CPU time
    QITI_REQUIRE(cpuTimeMs >= 0);

    // If ns value is >= 1,000,000 then ms should be at least 1
    if (cpuTimeNs >= 1000000)
    {
        QITI_REQUIRE(cpuTimeMs >= 1);
    }

    // ms should be consistent with ns (ms == ns / 1,000,000)
    QITI_REQUIRE(cpuTimeMs == cpuTimeNs / 1000000);
}

QITI_TEST_CASE("qiti::FunctionCallData::getTimeSpentInFunctionWallClock_ms()", FunctionCallDataGetTimeSpentWallClockMs)
{
    qiti::ScopedQitiTest test;

    auto funcData = qiti::FunctionData::getFunctionData<&slowWork>();
    QITI_REQUIRE(funcData != nullptr);

    // Warm up
    (void)slowWork();

    // Call to be tested
    (void)slowWork();

    auto lastCall = funcData->getLastFunctionCall();

    auto wallClockMs = lastCall.getTimeSpentInFunctionWallClock_ms();
    auto wallClockNs = lastCall.getTimeSpentInFunctionWallClock_ns();

    // slowWork does 100,000 iterations, should take at least 1ms of wall-clock time
    QITI_REQUIRE(wallClockMs >= 0);

    // If ns value is >= 1,000,000 then ms should be at least 1
    if (wallClockNs >= 1000000)
    {
        QITI_REQUIRE(wallClockMs >= 1);
    }

    // ms should be consistent with ns (ms == ns / 1,000,000)
    QITI_REQUIRE(wallClockMs == wallClockNs / 1000000);
}
#endif // ! _WIN32
