
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

//--------------------------------------------------------------------------

using namespace qiti::example::FunctionCallData;

// Disable optimizations for this entire file to prevent Release mode optimizations
// from interfering with number of or order of heap allocations
#pragma clang optimize off

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
        
        // Call the functions to be tested
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

// Re-enable optimizations for subsequent files
#pragma clang optimize on
