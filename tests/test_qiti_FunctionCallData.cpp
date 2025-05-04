
#include "qiti_FunctionCallData.hpp"
#include "qiti_FunctionData.hpp"
#include "qiti_utils.hpp"

#include "qiti_test_macros.hpp"

#include <catch2/catch_test_macros.hpp>

#pragma clang optimize off
/** NOT static to purposely allow external linkage and visibility to QITI */
void testHeapAllocationFunction() noexcept
{
    volatile int* test = new int{0};
    delete test;
}

/** NOT static to purposely allow external linkage and visibility to QITI */
int testNoHeapAllocationFunction() noexcept
{
    volatile int test{42};
    return test;
}
#pragma clang optimize on

TEST_CASE("qiti::FunctionCallData::getNumHeapAllocations() returns expected values")
{
    SECTION("1 heap allocation")
    {
        // Call twice
        testHeapAllocationFunction();
        testHeapAllocationFunction();
        
        auto funcData = qiti::getFunctionData<&testHeapAllocationFunction>();
        QITI_REQUIRE(funcData != nullptr);
        
        QITI_CHECK(funcData->getNumTimesCalled() == 2);
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        
        QITI_REQUIRE(lastFunctionCall.getNumHeapAllocations() == 1);
    }
    
    SECTION("0 heap allocation")
    {
        testNoHeapAllocationFunction();
        
        auto funcData = qiti::getFunctionData<&testNoHeapAllocationFunction>();
        QITI_REQUIRE(funcData != nullptr);
        
        QITI_CHECK(funcData->getNumTimesCalled() == 1);
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        
        QITI_REQUIRE(lastFunctionCall.getNumHeapAllocations() == 0);
    }
    
    qiti::shutdown();
}


