
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"
// Basic Catch2 macros
#include <catch2/catch_test_macros.hpp>

//--------------------------------------------------------------------------

using namespace qiti::example::FunctionCallData;

//--------------------------------------------------------------------------

TEST_CASE("qiti::FunctionCallData::getNumHeapAllocations()")
{
    qiti::ScopedQitiTest test;
    
    SECTION("1 heap allocations")
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
    
    SECTION("0 heap allocation")
    {
        auto funcData = qiti::FunctionData::getFunctionData<&testNoHeapAllocation>();
        QITI_REQUIRE(funcData != nullptr);
        
        testNoHeapAllocation();
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        QITI_REQUIRE(lastFunctionCall.getNumHeapAllocations() == 0);
    }
}

TEST_CASE("qiti::FunctionCallData::getAmountHeapAllocated()")
{
    qiti::ScopedQitiTest test;
    
    SECTION("1 heap allocations")
    {
        auto funcData = qiti::FunctionData::getFunctionData<&testHeapAllocation>();
        QITI_REQUIRE(funcData != nullptr);
        
        testHeapAllocation(); // contains 1 int allocaation
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        auto amountAllocated = lastFunctionCall.getAmountHeapAllocated();
        QITI_REQUIRE(amountAllocated == sizeof(int));
    }
}

TEST_CASE("qiti::FunctionCallData::getThreadThatCalledFunction()")
{
    qiti::ScopedQitiTest test;
    
    auto funcData = qiti::FunctionData::getFunctionData<&testHeapAllocation>();
    QITI_REQUIRE(funcData != nullptr);
    
    const auto currentThreadID = std::this_thread::get_id();
    
    SECTION("Called from unit test thread")
    {
        testHeapAllocation();
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        auto threadThatCalledFunction = lastFunctionCall.getThreadThatCalledFunction();
        QITI_REQUIRE(threadThatCalledFunction == currentThreadID); // called from this thread
    }
    
    SECTION("Called from custom thread")
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
