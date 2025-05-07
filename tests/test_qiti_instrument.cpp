
#include "qiti_include.hpp"

#include "qiti_test_macros.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("qiti::FunctionCallData::resetInstrumentation()")
{
    qiti::resetAll();
    
    static auto val = 0; // static required so that lambda can access it and still be converted to a function pointer
    
    // Set a heap allocation callback
    qiti::instrument::onNextHeapAllocation([](){++val;});
    
    // Clear out instrumentation which should heap allocation callback
    qiti::instrument::resetInstrumentation();
    
    // Explicit heap allocation, would normally trigger onNextHeapAllocation()
    volatile auto* testAlloc = new int{42};
    
    // Heap allocation callback should not have been run, therefore val should be unchanged
    QITI_CHECK(val == 0);
    
    // Cleanup
    delete testAlloc;
    qiti::resetAll();
}

TEST_CASE("qiti::onNextHeapAllocation() is called on next heap allocation", "[qiti::onNextHeapAllocation]")
{
    qiti::resetAll();
    
    static int testValue = 0;
    qiti::instrument::onNextHeapAllocation([](){ ++testValue; });
    
    auto* heapAllocation = new int{0};
    QITI_CHECK(testValue == 1);
    delete heapAllocation;
    
    qiti::resetAll();
}

TEST_CASE("qiti::FunctionCallData::assertOnNextHeapAllocation()")
{
    qiti::resetAll();
    
    
    qiti::resetAll();
}
