
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

#include <catch2/catch_test_macros.hpp>


TEST_CASE("qiti::FunctionCallData::getNumHeapAllocations() returns expected values")
{
    qiti::resetAll();
    qiti::profile::beginProfilingAllFunctions();
    
    SECTION("1 heap allocation")
    {
        // Call twice
        qiti::example::testHeapAllocationFunction();
        qiti::example::testHeapAllocationFunction();
        
        auto funcData = qiti::getFunctionData<&qiti::example::testHeapAllocationFunction>();
        QITI_REQUIRE(funcData != nullptr);
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        QITI_REQUIRE(lastFunctionCall.getNumHeapAllocations() == 1);
    }
    
    SECTION("0 heap allocation")
    {
        qiti::example::testNoHeapAllocationFunction();
        
        auto funcData = qiti::getFunctionData<&qiti::example::testNoHeapAllocationFunction>();
        QITI_REQUIRE(funcData != nullptr);
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        QITI_REQUIRE(lastFunctionCall.getNumHeapAllocations() == 0);
    }
    
    qiti::resetAll();
}

