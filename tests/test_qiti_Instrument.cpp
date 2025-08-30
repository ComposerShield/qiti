
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

//--------------------------------------------------------------------------
// Function call instrumentation tests
//--------------------------------------------------------------------------

// Test function for onNextFunctionCall tests
__attribute__((noinline))
__attribute__((optnone))
static void testTargetFunction()
{
    // Empty function for testing purposes
    [[maybe_unused]] volatile int dummyInt = 42;
}

QITI_TEST_CASE("qiti::Instrument::onNextFunctionCall() is called on next function call", OnNextFunctionCallIsCalled)
{
    qiti::ScopedQitiTest test;
    
    int callbackCount = 0;
    auto incrementCallback = [&callbackCount]()
    {
        ++callbackCount;
    };
    
    // Set up callback for testTargetFunction
    qiti::Instrument::onNextFunctionCall<testTargetFunction>(incrementCallback);
    
    // Call the function - should trigger callback
    testTargetFunction();
    
    // Callback should have been executed once
    QITI_CHECK(callbackCount == 1);
    
    // Second call should not trigger callback (one-time only)
    testTargetFunction();
    QITI_CHECK(callbackCount == 1);
}

QITI_TEST_CASE("qiti::Instrument::resetInstrumentation() clears function call hooks", ResetInstrumentationClearsFunctionCallHooks)
{
    qiti::ScopedQitiTest test;
    
    int callbackCount = 0;
    auto incrementCallback = [&callbackCount]() { ++callbackCount; };
    
    // Set up callback
    qiti::Instrument::onNextFunctionCall<&testTargetFunction>(incrementCallback);
    
    // Reset instrumentation should clear the callback
    qiti::Instrument::resetInstrumentation();
    
    // Call function - callback should not execute
    testTargetFunction();
    
    // Callback should not have been executed
    QITI_CHECK(callbackCount == 0);
}
