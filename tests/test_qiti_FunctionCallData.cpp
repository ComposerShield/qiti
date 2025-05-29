
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"
// Basic Catch2 macros
#include <catch2/catch_test_macros.hpp>

using namespace qiti::example::FunctionCallData;

#if defined(__APPLE__) // TODO: support Linux
TEST_CASE("qiti::FunctionCallData::getNumHeapAllocations() returns expected values")
{
    qiti::ScopedQitiTest test;
    
    auto funcData = qiti::getFunctionData<&testHeapAllocation>();
    QITI_REQUIRE(funcData != nullptr);
    
    SECTION("1 heap allocation")
    {
        // Call twice
        testHeapAllocation();
        testHeapAllocation();
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        QITI_REQUIRE(lastFunctionCall.getNumHeapAllocations() == 1);
    }
    
    SECTION("0 heap allocation")
    {
        testNoHeapAllocation();
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        QITI_REQUIRE(lastFunctionCall.getNumHeapAllocations() == 0);
    }
}
#endif // defined(__APPLE__)
