
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

// Qiti Private API - not included in qiti_include.hpp
#include "qiti_Instrument.hpp"

//--------------------------------------------------------------------------

QITI_TEST_CASE("qiti::FunctionCallData::resetInstrumentation()", FunctionCallDataResetInstrumentation)
{
    qiti::ScopedQitiTest test;
    
    static auto val = 0; // static required so that lambda can access it and still be converted to a function pointer
    
    // Set a heap allocation callback
    qiti::Instrument::onNextHeapAllocation([](){++val;});
    
    // Clear out instrumentation which should heap allocation callback
    qiti::Instrument::resetInstrumentation();
    
    // Explicit heap allocation, would normally trigger onNextHeapAllocation()
    volatile auto* testAlloc = new int{42};
    
    // Heap allocation callback should not have been run, therefore val should be unchanged
    QITI_CHECK(val == 0);
    
    // Cleanup
    delete testAlloc;
}

// Disable optimizations for heap allocation tests to prevent Release mode optimizations
// from interfering with timing (and existance) of heap allocations
#pragma clang optimize off

QITI_TEST_CASE("qiti::onNextHeapAllocation() is called on next heap allocation", OnNextHeapAllocationIsCalled)
{
    qiti::ScopedQitiTest test;
    
    static int testValue = 0;
    qiti::Instrument::onNextHeapAllocation([](){ ++testValue; });
    
    // Volatile to ensure compiler does not reorder it and break check
    volatile auto* heapAllocation = new int{0};
    QITI_CHECK(testValue == 1);
    delete heapAllocation;
}

// Re-enable optimizations for subsequent files
#pragma clang optimize on
