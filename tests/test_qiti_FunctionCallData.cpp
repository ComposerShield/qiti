
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

//--------------------------------------------------------------------------

using namespace qiti::example::FunctionCallData;

//--------------------------------------------------------------------------

QITI_TEST_CASE("qiti::FunctionCallData::getNumHeapAllocations()")
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

QITI_TEST_CASE("qiti::FunctionCallData::getAmountHeapAllocated()")
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

QITI_TEST_CASE("qiti::FunctionCallData::getThreadThatCalledFunction()")
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

QITI_TEST_CASE("qiti::FunctionCallData::getTimeSpentInFunction")
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
    
    QITI_SECTION("More work results in more time elapsed")
    {
        auto someWorkFuncData = qiti::FunctionData::getFunctionData<&someWork>();
        QITI_REQUIRE(someWorkFuncData != nullptr);
        
        auto moreWorkFuncData = qiti::FunctionData::getFunctionData<&moreWork>();
        QITI_REQUIRE(moreWorkFuncData != nullptr);
        
        // Call once to "warm up" CPU to these functions.
        // Else first function always gets additional performance hit and the results are less predictable.
        // The CPU needs to fetch the code into its I-cache, train its predictors, lazy-load the pages, etc.
        (void)someWork(); // "cold call"
        (void)moreWork(); // "cold call"
        
        // Calls to be tested below
        (void)someWork();
        (void)moreWork();
        
        auto someWorkLastFunctionCall = someWorkFuncData->getLastFunctionCall();
        auto moreWorkLastFunctionCall = moreWorkFuncData->getLastFunctionCall();
        
        auto someWorkTimeSpentCpu   = someWorkLastFunctionCall.getTimeSpentInFunctionCpu_ns();
        auto moreWorkTimeSpentCpu   = moreWorkLastFunctionCall.getTimeSpentInFunctionCpu_ns();
        
        auto someWorkTimeSpentClock = someWorkLastFunctionCall.getTimeSpentInFunctionWallClock_ns();
        auto moreWorkTimeSpentClock = moreWorkLastFunctionCall.getTimeSpentInFunctionWallClock_ns();
        
        QITI_REQUIRE(moreWorkTimeSpentCpu >= someWorkTimeSpentCpu);
        QITI_REQUIRE(moreWorkTimeSpentClock >= someWorkTimeSpentClock);
    }
}
