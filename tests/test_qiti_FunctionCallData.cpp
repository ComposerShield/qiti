
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace qiti::example::FunctionCallData;

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

