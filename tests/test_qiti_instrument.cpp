
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"
// Basic Catch2 macros
#include <catch2/catch_test_macros.hpp>

// Qiti Private API - not included in qiti_include.hpp
#include "qiti_instrument.hpp"

//--------------------------------------------------------------------------

TEST_CASE("qiti::FunctionCallData::resetInstrumentation()")
{
    qiti::ScopedQitiTest test;
    
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
}

#if defined(__APPLE__) // TODO: support Linux
TEST_CASE("qiti::onNextHeapAllocation() is called on next heap allocation", "[qiti::onNextHeapAllocation]")
{
    qiti::ScopedQitiTest test;
    
    static int testValue = 0;
    qiti::instrument::onNextHeapAllocation([](){ ++testValue; });
    
    auto* heapAllocation = new int{0};
    QITI_CHECK(testValue == 1);
    delete heapAllocation;
}
#endif // defined(__APPLE__)
