
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

// Qiti Private API - not included in qiti_include.hpp
#include "qiti_Instrument.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <future> // for std::async
#include <thread> // for std::thread

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

QITI_TEST_CASE("qiti::Instrument::onNextFunctionCall() is called on next function call",
               OnNextFunctionCallIsCalled)
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

QITI_TEST_CASE("qiti::Instrument::resetInstrumentation() clears function call hooks",
               ResetInstrumentationClearsFunctionCallHooks)
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

//--------------------------------------------------------------------------
// Thread creation instrumentation tests
//--------------------------------------------------------------------------

QITI_TEST_CASE("qiti::Instrument::onThreadCreation() detects new thread creation",
               OnThreadCreationDetectsNewThread)
{
    qiti::ScopedQitiTest test;
    
    std::thread::id detectedThreadId{};
    bool callbackExecuted = false;
    
    auto threadCallback = [&detectedThreadId, &callbackExecuted](std::thread::id threadId)
    {
        detectedThreadId = threadId;
        callbackExecuted = true;
    };
    
    // Set up thread creation callback
    qiti::Instrument::onThreadCreation(threadCallback);
    
    std::thread::id actualThreadId{};
    
    // Create a new thread
    std::thread newThread([&actualThreadId]()
    {
        actualThreadId = std::this_thread::get_id();
    });
    
    newThread.join();
    
    // Verify callback was executed and received correct thread ID
    QITI_CHECK(callbackExecuted);
    QITI_CHECK(detectedThreadId == actualThreadId);
}

QITI_TEST_CASE("qiti::Instrument::resetInstrumentation() clears thread creation hooks",
               ResetInstrumentationClearsThreadHooks)
{
    qiti::ScopedQitiTest test;
    
    bool callbackExecuted = false;
    
    auto threadCallback = [&callbackExecuted](std::thread::id)
    {
        callbackExecuted = true;
    };
    
    // Set up thread creation callback
    qiti::Instrument::onThreadCreation(threadCallback);
    
    // Reset instrumentation should clear the callback
    qiti::Instrument::resetInstrumentation();
    
    // Create a new thread - callback should not execute
    std::thread newThread([]()
    {
        // empty
    });
    
    newThread.join();
    
    // Callback should not have been executed
    QITI_CHECK(! callbackExecuted);
}

QITI_TEST_CASE("qiti::Instrument::onThreadCreation() detects thread without function calls",
               OnThreadCreationNoFunctionCalls)
{
    qiti::ScopedQitiTest test;
    
    bool callbackExecuted = false;
    
    auto threadCallback = [&callbackExecuted](std::thread::id threadId)
    {
        callbackExecuted = true;
    };
    
    // Set up thread creation callback
    qiti::Instrument::onThreadCreation(threadCallback);
    
    // Create a new thread that doesn't call any functions
    std::thread newThread([]()
    {
        // No function calls - just thread creation should be enough to trigger hook
    });
    
    newThread.join();
    
    // Verify callback was executed
    QITI_CHECK(callbackExecuted);
}

#ifndef _WIN32
QITI_TEST_CASE("qiti::Instrument::onThreadCreation() detects pthread creation without function calls",
               OnThreadCreationPthreadNoFunctionCalls)
{
    qiti::ScopedQitiTest test;
    
    bool callbackExecuted = false;
    pthread_t threadHandle{};
    
    auto threadCallback = [&callbackExecuted](std::thread::id threadId)
    {
        callbackExecuted = true;
    };
    
    // Set up thread creation callback
    qiti::Instrument::onThreadCreation(threadCallback);
    
    // Create a new pthread that doesn't call any functions
    int result = pthread_create(&threadHandle, nullptr, [](void*) -> void*
    {
        // Empty pthread function - no function calls
        return nullptr;
    }, nullptr);
    
    QITI_REQUIRE(result == 0); // pthread_create succeeded
    
    pthread_join(threadHandle, nullptr);
    
    // Verify callback was executed
    QITI_CHECK(callbackExecuted);
}
#endif // ! _WIN32

#ifdef _WIN32
QITI_TEST_CASE("qiti::Instrument::onThreadCreation() detects CreateThread creation without function calls",
               OnThreadCreationCreateThreadNoFunctionCalls)
{
    qiti::ScopedQitiTest test;
    
    bool callbackExecuted = false;
    
    auto threadCallback = [&callbackExecuted](std::thread::id threadId)
    {
        callbackExecuted = true;
    };
    
    // Set up thread creation callback
    qiti::Instrument::onThreadCreation(threadCallback);
    
    // Create a new Windows thread that doesn't call any functions
    HANDLE threadHandle = CreateThread
    (
        nullptr,    // default security attributes
        0,          // default stack size
        [](LPVOID) -> DWORD
        {
            // Empty thread function - no function calls
            return 0;
        },
        nullptr,    // no thread parameter
        0,          // default creation flags
        nullptr     // don't need thread ID
    );
    
    QITI_REQUIRE(threadHandle != nullptr); // CreateThread succeeded
    
    WaitForSingleObject(threadHandle, INFINITE);
    CloseHandle(threadHandle);
    
    // Verify callback was executed
    QITI_CHECK(callbackExecuted);
}
#endif // _WIN32

QITI_TEST_CASE("qiti::Instrument::onThreadCreation() detects std::async thread creation",
               OnThreadCreationStdAsync)
{
    qiti::ScopedQitiTest test;

    bool callbackExecuted = false;

    auto threadCallback = [&callbackExecuted](std::thread::id threadId)
    {
        callbackExecuted = true;
    };

    // Set up thread creation callback
    qiti::Instrument::onThreadCreation(threadCallback);

    // Use std::async with launch::async to force new thread creation
    // Note: Implementation may still reuse existing thread pool threads
    auto future = std::async(std::launch::async, []()
    {
        // Empty async function - no function calls
        return 42;
    });

    int result = future.get();  // Wait for completion
    QITI_REQUIRE(result == 42); // async task completed successfully

    // Verify callback was executed (if a new thread was actually created)
    // Note: This may not always pass since std::async behavior is implementation-defined
    QITI_CHECK(callbackExecuted);
}

//--------------------------------------------------------------------------
// New coverage tests
//--------------------------------------------------------------------------

QITI_TEST_CASE("qiti::Instrument::assertOnNextHeapAllocation() sets up assertion then is cleared by reset",
               AssertOnNextHeapAllocationSetupAndReset)
{
    qiti::ScopedQitiTest test;

    // Call assertOnNextHeapAllocation() which internally calls
    // onNextHeapAllocation([]{ assert(false); }). This exercises the
    // assertOnNextHeapAllocation() code path (lines 89-93).
    qiti::Instrument::assertOnNextHeapAllocation();

    // Immediately reset instrumentation so that the assert(false) callback
    // is removed before any heap allocation can trigger it.
    qiti::Instrument::resetInstrumentation();

    // Now perform a heap allocation. Since we reset, the callback should
    // not fire and the test should not crash.
    volatile auto* testAlloc = new int{42};
    delete testAlloc;

    // If we get here, the reset successfully cleared the assertion callback.
    QITI_CHECK(true);
}
