
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
