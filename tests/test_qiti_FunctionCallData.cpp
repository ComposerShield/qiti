
#include "qiti_include.hpp"

#include "qiti_test_macros.hpp"

#include <catch2/catch_test_macros.hpp>

/** NOT static to purposely allow external linkage and visibility to QITI */
__attribute__((noinline)) __attribute__((optnone))
void testHeapAllocationFunction() noexcept
{
    volatile int* test = new int{0};
    delete test;
}

/** NOT static to purposely allow external linkage and visibility to QITI */
__attribute__((noinline)) __attribute__((optnone))
int testNoHeapAllocationFunction() noexcept
{
    volatile int test{42};
    return test;
}

TEST_CASE("qiti::FunctionCallData::getNumHeapAllocations() returns expected values")
{
    qiti::resetAll();
    qiti::profile::beginProfilingAllFunctions();
    
    SECTION("1 heap allocation")
    {
        // Call twice
        testHeapAllocationFunction();
        testHeapAllocationFunction();
        
        auto funcData = qiti::getFunctionData<&testHeapAllocationFunction>();
        QITI_REQUIRE(funcData != nullptr);
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        QITI_REQUIRE(lastFunctionCall.getNumHeapAllocations() == 1);
    }
    
    SECTION("0 heap allocation")
    {
        testNoHeapAllocationFunction();
        
        auto funcData = qiti::getFunctionData<&testNoHeapAllocationFunction>();
        QITI_REQUIRE(funcData != nullptr);
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        QITI_REQUIRE(lastFunctionCall.getNumHeapAllocations() == 0);
    }
    
    qiti::resetAll();
}


