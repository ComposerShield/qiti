
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"
// Basic Catch2 macros
#include <catch2/catch_test_macros.hpp>

#include <memory>

[[maybe_unused]] inline static std::allocator<char> fixLinkerError;

using namespace qiti::example::FunctionCallData;

TEST_CASE("qiti::FunctionCallData::getNumHeapAllocations() returns expected values")
{
    qiti::ScopedQitiTest test;
    qiti::profile::beginProfilingAllFunctions();
    
    qiti::profile::beginProfilingFunction<testHeapAllocation>();
    qiti::profile::beginProfilingFunction<testNoHeapAllocation>();
    
    SECTION("1 heap allocation")
    {
        // Call twice
        testHeapAllocation();
        testHeapAllocation();
        
        auto funcData = qiti::getFunctionData<&testHeapAllocation>();
        QITI_REQUIRE(funcData != nullptr);
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        QITI_REQUIRE(lastFunctionCall.getNumHeapAllocations() == 1);
    }
    
    SECTION("0 heap allocation")
    {
        testNoHeapAllocation();
        
        auto funcData = qiti::getFunctionData<&testNoHeapAllocation>();
        QITI_REQUIRE(funcData != nullptr);
        
        auto lastFunctionCall = funcData->getLastFunctionCall();
        QITI_REQUIRE(lastFunctionCall.getNumHeapAllocations() == 0);
    }
    
    qiti::profile::endProfilingAllFunctions();
}

