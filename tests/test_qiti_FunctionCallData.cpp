
#include "qiti_include.hpp"

#include "qiti_test_macros.hpp"

#include <catch2/catch_test_macros.hpp>

#include <iostream>

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
    std::cout << qiti::profile::getNumHeapAllocations() << "\n";
    qiti::resetAll();
    std::cout << qiti::profile::getNumHeapAllocations() << "\n";
    qiti::profile::beginProfilingAllFunctions();
    std::cout << qiti::profile::getNumHeapAllocations() << "\n";
    
    SECTION("2 heap allocations")
    {
        qiti::resetAll();
        std::cout << qiti::profile::getNumHeapAllocations() << "\n";
        
        // Call twice
        testHeapAllocationFunction();
        std::cout << qiti::profile::getNumHeapAllocations() << "\n";
        testHeapAllocationFunction();
        std::cout << qiti::profile::getNumHeapAllocations() << "\n";
        
        auto funcData = qiti::getFunctionData<&testHeapAllocationFunction>();
        std::cout << qiti::profile::getNumHeapAllocations() << "\n";
        QITI_REQUIRE(funcData != nullptr);
        
        std::cout << qiti::profile::getNumHeapAllocations() << "\n";
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        auto num = lastFunctionCall.getNumHeapAllocations();
        QITI_REQUIRE(num == 2);
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


