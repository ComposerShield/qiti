
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

#include <algorithm>
#include <random>
#include <thread>
#include <vector>

#include <iostream>

#ifdef _WIN32
[[maybe_unused]] extern bool special_debug;
#endif
//--------------------------------------------------------------------------

/** NOT static to purposely allow external linkage and visibility to QITI */
__attribute__((noinline))
__attribute__((optnone))
void testFunc() noexcept
{
    volatile int _ = 42;
}

/** Test function with variable execution time for min/max testing */
__attribute__((noinline))
__attribute__((optnone))
void testFuncWithVariableLength(int relativeLength) noexcept
{
    volatile int sum = 0;
    // Create variable execution time by doing different amounts of work
#ifdef _WIN32
    // Windows: Need more work to get measurable CPU time due to limited timer resolution
    int multiplier = 50000;  // Increased work load for Windows
#else
    int multiplier = 1000;   // Work load for Unix systems
#endif
    for(int i = 0; i < relativeLength * multiplier; ++i)
        sum = sum + i;
}

/** Test functions for caller tracking */
__attribute__((noinline))
__attribute__((optnone))
void callerTestFuncA() noexcept
{
    volatile int _ = 1;
}

__attribute__((noinline))
__attribute__((optnone))
void callerTestFuncB() noexcept
{
    callerTestFuncA(); // B calls A
}

__attribute__((noinline))
__attribute__((optnone))
void callerTestFuncC() noexcept
{
    callerTestFuncA(); // C also calls A
    callerTestFuncB(); // C calls B (which calls A)
}

__attribute__((noinline))
__attribute__((optnone))
void testFuncThrowsException()
{
    throw std::runtime_error("Test exception");
}

__attribute__((noinline))
__attribute__((optnone))
void testFuncCatchesException()
{
    try
    {
        testFuncThrowsException();
    }
    catch (const std::exception&)
    {
        // Caught the exception
    }
}

__attribute__((noinline))
__attribute__((optnone))
void testFuncThrowsUncaughtException()
{
    testFuncThrowsException(); // This will propagate
}

// Test classes for function type detection
class TestClassForFunctionTypes
{
public:
    __attribute__((noinline))
    __attribute__((optnone))
    TestClassForFunctionTypes() : value(0) {}        // Regular constructor
    
    __attribute__((noinline))
    __attribute__((optnone))
    TestClassForFunctionTypes(const TestClassForFunctionTypes& other) : value(other.value) {}  // Copy constructor
    
    __attribute__((noinline))
    __attribute__((optnone))
    TestClassForFunctionTypes(TestClassForFunctionTypes&& other) : value(other.value)
    {
        other.value = -1;  // Move constructor
    }
    
    __attribute__((noinline))
    __attribute__((optnone))
    TestClassForFunctionTypes& operator=(const TestClassForFunctionTypes& other)  // Copy assignment
    {
        if (this != &other)
            value = other.value;
        return *this;
    }
    
    __attribute__((noinline))
    __attribute__((optnone))
    TestClassForFunctionTypes& operator=(TestClassForFunctionTypes&& other)  // Move assignment
    {
        if (this != &other)
        {
            value = other.value;
            other.value = -1;
        }
        return *this;
    }
    
    __attribute__((noinline))
    __attribute__((optnone))
    ~TestClassForFunctionTypes() {} // Destructor
    
    __attribute__((noinline))
    __attribute__((optnone))
    void regularMethod() {} // Regular method
    
private:
    int value; // dummy value
};

//--------------------------------------------------------------------------

QITI_TEST_CASE("qiti::FunctionData::getFunctionName()", FunctionDataGetFunctionName)
{
    qiti::ScopedQitiTest test;

    {
        qiti::Profile::beginProfilingFunction<&testFunc>();
        auto functionData = qiti::Utils::getFunctionData<&testFunc>();
        std::string name = functionData->getFunctionName();
#ifdef _WIN32
        // On Windows, allow either the exact name or a name containing testFunc
        QITI_CHECK((name == "testFunc" || name.find("testFunc") != std::string::npos));
#else
        QITI_CHECK(name == "testFunc");
#endif
    }
}

QITI_TEST_CASE("qiti::FunctionData::getNumTimesCalled()", FunctionDataGetNumTimesCalled)
{
    qiti::ScopedQitiTest test;
    
    qiti::Profile::beginProfilingFunction<&testFunc>();
    
    auto funcData = qiti::Utils::getFunctionData<&testFunc>();
    QITI_REQUIRE(funcData != nullptr);
    
    QITI_SECTION("Called twice")
    {
        testFunc();
        testFunc();
        QITI_CHECK(funcData->getNumTimesCalled() == 2);
    }
    
    QITI_SECTION("Not called")
    {
        QITI_CHECK(funcData->getNumTimesCalled() == 0);
    }
    
    QITI_SECTION("Called twice on two different threads")
    {
        std::thread t([]{ testFunc(); });
        testFunc();
        t.join();
        QITI_CHECK(funcData->getNumTimesCalled() == 2);
    }
}

QITI_TEST_CASE("qiti::FunctionData::getNumTimesCalled(), using static constructor", FunctionDataGetNumTimesCalledStaticConstructor)
{
    qiti::ScopedQitiTest test;
    
    auto funcData = qiti::FunctionData::getFunctionData<&testFunc>();
    QITI_REQUIRE(funcData != nullptr);
    
    QITI_SECTION("Called once")
    {
        testFunc();
        QITI_CHECK(funcData->getNumTimesCalled() == 1);
    }
    
    QITI_SECTION("Not called")
    {
        QITI_CHECK(funcData->getNumTimesCalled() == 0);
    }
}

QITI_TEST_CASE("qiti::FunctionData::wasCalledOnThread()", FunctionDataWasCalledOnThread)
{
    qiti::ScopedQitiTest test;
    
    qiti::Profile::beginProfilingFunction<&testFunc>();
    
    auto funcData = qiti::Utils::getFunctionData<&testFunc>();
    QITI_REQUIRE(funcData != nullptr);
    
    QITI_SECTION("Function called on current thread")
    {
        testFunc();
        
        std::thread::id currentThread = std::this_thread::get_id();
        
        QITI_CHECK(funcData->wasCalledOnThread(currentThread));
    }
    
    QITI_SECTION("Function never called")
    {
        std::thread::id currentThread = std::this_thread::get_id();
        
        QITI_CHECK(! funcData->wasCalledOnThread(currentThread));
    }
    
    QITI_SECTION("Function called on custom thread")
    {
        std::thread thread([]{ testFunc(); });
        auto id = thread.get_id();
        thread.join(); // sync with thread to ensure function was called
        
        QITI_CHECK(funcData->wasCalledOnThread(id));
    }
}

QITI_TEST_CASE("qiti::FunctionData::getAllProfiledFunctionData()", FunctionDataGetAllProfiledFunctionData)
{
    qiti::ScopedQitiTest test;
    
    // Profile two functions
    qiti::Profile::beginProfilingFunction<&testFunc>();
    qiti::Profile::beginProfilingFunction<&qiti::example::FunctionCallData::testHeapAllocation>();
    
    // Run the two functions
    testFunc();
    qiti::example::FunctionCallData::testHeapAllocation();
    
    // getAllProfiledFunctionData() contains our two functions
    {
        auto numHeapAllocsBefore = qiti::Profile::getNumHeapAllocationsOnCurrentThread();
        auto allFunctions = qiti::FunctionData::getAllProfiledFunctionData();
        QITI_REQUIRE(allFunctions.size() >= 2);
        QITI_REQUIRE(numHeapAllocsBefore == qiti::Profile::getNumHeapAllocationsOnCurrentThread());
        
        auto containsFunc = [&allFunctions](const std::string& funcName)->bool
        {
            for (const auto* func : allFunctions)
                if (std::string(func->getFunctionName()) == funcName)
                    return true;
            return false;
        };
        
        QITI_CHECK(containsFunc("testFunc"));
        QITI_CHECK(containsFunc("qiti::example::FunctionCallData::testHeapAllocation"));
        QITI_REQUIRE_FALSE(containsFunc("randomFuncNameThatWeDidNotCall"));
    }
    
    // Reset
    test.reset(false);
    
    // No profiled functions should be available
    auto allFunctionsAfterReset = qiti::FunctionData::getAllProfiledFunctionData();
    QITI_REQUIRE(allFunctionsAfterReset.size() == 0);
}

QITI_TEST_CASE("qiti::FunctionData::getMinTimeSpentInFunctionCpu_ns()", FunctionDataGetMinTimeSpentInFunctionCpu)
{
    qiti::ScopedQitiTest test;
    
    auto funcData = qiti::FunctionData::getFunctionData<&testFuncWithVariableLength>();
    QITI_REQUIRE(funcData != nullptr);
    
    QITI_SECTION("No calls made - should return 0")
    {
        QITI_CHECK(funcData->getMinTimeSpentInFunctionCpu_ns() == 0);
    }
    
    QITI_SECTION("Single call")
    {
        special_debug = true;
        testFuncWithVariableLength(1); // Small delay
        special_debug = false;
        
        uint64_t minTime = funcData->getMinTimeSpentInFunctionCpu_ns();
        QITI_CHECK(minTime > 0); // Should have some measurable time
    }
    
    QITI_SECTION("Multiple calls with different execution times")
    {
        // Make calls with different execution times
        testFuncWithVariableLength(5);  // Longer delay
        testFuncWithVariableLength(1);  // Shorter delay  
        testFuncWithVariableLength(3);  // Medium delay
        
        uint64_t minTime = funcData->getMinTimeSpentInFunctionCpu_ns();
        uint64_t maxTime = funcData->getMaxTimeSpentInFunctionCpu_ns();
        
        QITI_CHECK(minTime > 0);
        QITI_CHECK(maxTime > 0);
        QITI_CHECK(minTime <= maxTime); // Min should be <= max
        QITI_CHECK(funcData->getNumTimesCalled() == 3);
    }
}

QITI_TEST_CASE("qiti::FunctionData::getMaxTimeSpentInFunctionCpu_ns()", FunctionDataGetMaxTimeSpentInFunctionCpu)
{
    qiti::ScopedQitiTest test;
    
    auto funcData = qiti::FunctionData::getFunctionData<&testFuncWithVariableLength>();
    QITI_REQUIRE(funcData != nullptr);
    
    QITI_SECTION("No calls made - should return 0")
    {
        QITI_CHECK(funcData->getMaxTimeSpentInFunctionCpu_ns() == 0);
    }
    
    QITI_SECTION("Single call")
    {
        testFuncWithVariableLength(1); // Small delay
        uint64_t maxTime = funcData->getMaxTimeSpentInFunctionCpu_ns();
        QITI_CHECK(maxTime > 0); // Should have some measurable time
    }
    
    QITI_SECTION("Multiple calls - max should track longest execution")
    {
        testFuncWithVariableLength(1);  // Short
        uint64_t timeAfterFirst = funcData->getMaxTimeSpentInFunctionCpu_ns();
        
        testFuncWithVariableLength(50);  // Longer - should become new max
        uint64_t timeAfterSecond = funcData->getMaxTimeSpentInFunctionCpu_ns();
        
        testFuncWithVariableLength(2);  // Medium - shouldn't change max
        uint64_t timeAfterThird = funcData->getMaxTimeSpentInFunctionCpu_ns();
        
        QITI_CHECK(timeAfterFirst > 0);
        QITI_CHECK(timeAfterSecond >= timeAfterFirst); // Second call was longer
        QITI_CHECK(timeAfterThird == timeAfterSecond); // Third call shouldn't change max
        QITI_CHECK(funcData->getNumTimesCalled() == 3);
    }
}

QITI_TEST_CASE("qiti::FunctionData::getMinTimeSpentInFunctionWallClock_ns()", FunctionDataGetMinTimeSpentInFunctionWallClock)
{
    qiti::ScopedQitiTest test;
    
    auto funcData = qiti::FunctionData::getFunctionData<&testFuncWithVariableLength>();
    QITI_REQUIRE(funcData != nullptr);
    
    QITI_SECTION("No calls made - should return 0")
    {
        QITI_CHECK(funcData->getMinTimeSpentInFunctionWallClock_ns() == 0);
    }
    
    QITI_SECTION("Multiple calls with different execution times")
    {
        testFuncWithVariableLength(3);  // Medium delay
        testFuncWithVariableLength(1);  // Shorter delay
        testFuncWithVariableLength(5);  // Longer delay
        
        uint64_t minTime = funcData->getMinTimeSpentInFunctionWallClock_ns();
        uint64_t maxTime = funcData->getMaxTimeSpentInFunctionWallClock_ns();
        
        QITI_CHECK(minTime > 0);
        QITI_CHECK(maxTime > 0);
        QITI_CHECK(minTime <= maxTime); // Min should be <= max
        QITI_CHECK(funcData->getNumTimesCalled() == 3);
    }
}

QITI_TEST_CASE("qiti::FunctionData::getMaxTimeSpentInFunctionWallClock_ns()", FunctionDataGetMaxTimeSpentInFunctionWallClock)
{
    qiti::ScopedQitiTest test;
    
    auto funcData = qiti::FunctionData::getFunctionData<&testFuncWithVariableLength>();
    QITI_REQUIRE(funcData != nullptr);
    
    QITI_SECTION("No calls made - should return 0")
    {
        QITI_CHECK(funcData->getMaxTimeSpentInFunctionWallClock_ns() == 0);
    }
    
    QITI_SECTION("Single call")
    {
        testFuncWithVariableLength(2);
        uint64_t maxTime = funcData->getMaxTimeSpentInFunctionWallClock_ns();
        QITI_CHECK(maxTime > 0); // Should have some measurable time
    }
    
#ifndef _WIN32
    QITI_SECTION("CPU vs WallClock consistency check")
    {
        testFuncWithVariableLength(2);
        testFuncWithVariableLength(4);
        
        uint64_t minCpu = funcData->getMinTimeSpentInFunctionCpu_ns();
        uint64_t maxCpu = funcData->getMaxTimeSpentInFunctionCpu_ns();
        uint64_t minWall = funcData->getMinTimeSpentInFunctionWallClock_ns();
        uint64_t maxWall = funcData->getMaxTimeSpentInFunctionWallClock_ns();
        
        // Both timing methods should show that min <= max
        QITI_CHECK(minCpu <= maxCpu);
        QITI_CHECK(minWall <= maxWall);
        QITI_CHECK(minCpu > 0);
        QITI_CHECK(minWall > 0);
        QITI_CHECK(funcData->getNumTimesCalled() == 2);
    }
#endif // ! _WIN32
}

QITI_TEST_CASE("qiti::FunctionCallData::getCaller()", FunctionCallDataGetCaller)
{
    qiti::ScopedQitiTest test;
    
    // Enable profiling for all functions for reliable caller tracking
    qiti::Profile::beginProfilingAllFunctions();
    
    auto funcDataA = qiti::FunctionData::getFunctionData<&callerTestFuncA>();
    auto funcDataB = qiti::FunctionData::getFunctionData<&callerTestFuncB>();
    auto funcDataC = qiti::FunctionData::getFunctionData<&callerTestFuncC>();
    
    QITI_REQUIRE(funcDataA != nullptr);
    QITI_REQUIRE(funcDataB != nullptr);
    QITI_REQUIRE(funcDataC != nullptr);
    
    QITI_SECTION("Direct call - no caller")
    {
        callerTestFuncA(); // Called directly from test
        auto lastCall = funcDataA->getLastFunctionCall();
        QITI_CHECK(lastCall.getCaller() == nullptr); // No profiled caller
    }
    
    QITI_SECTION("Function called by another function")
    {
        callerTestFuncB(); // B calls A
        auto lastCallA = funcDataA->getLastFunctionCall();
        QITI_CHECK(lastCallA.getCaller() == funcDataB); // A was called by B
    }
    
    QITI_SECTION("Complex call chain")
    {
        callerTestFuncC(); // C calls A, then C calls B (which calls A)
        
        // After this call sequence:
        // - A was last called by B (since C->B->A was the final call to A)
        // - B was called by C
        auto lastCallA = funcDataA->getLastFunctionCall();
        auto lastCallB = funcDataB->getLastFunctionCall();
        
        QITI_CHECK(lastCallA.getCaller() == funcDataB); // A's last caller was B
        QITI_CHECK(lastCallB.getCaller() == funcDataC); // B was called by C
    }
}

QITI_TEST_CASE("qiti::FunctionData::getCallers()", FunctionDataGetCallers)
{
    qiti::ScopedQitiTest test;
    
    // Enable profiling for all functions for reliable caller tracking
    qiti::Profile::beginProfilingAllFunctions();
    
    auto funcDataA = qiti::FunctionData::getFunctionData<&callerTestFuncA>();
    auto funcDataB = qiti::FunctionData::getFunctionData<&callerTestFuncB>();
    auto funcDataC = qiti::FunctionData::getFunctionData<&callerTestFuncC>();
    
    QITI_REQUIRE(funcDataA != nullptr);
    QITI_REQUIRE(funcDataB != nullptr);
    QITI_REQUIRE(funcDataC != nullptr);
    
    QITI_SECTION("No callers initially")
    {
        auto callersA = funcDataA->getCallers();
        auto callersB = funcDataB->getCallers(); 
        auto callersC = funcDataC->getCallers();
        
        QITI_CHECK(callersA.empty());
        QITI_CHECK(callersB.empty());
        QITI_CHECK(callersC.empty());
    }
    
    QITI_SECTION("Single caller")
    {
        callerTestFuncB(); // B calls A
        
        auto callersA = funcDataA->getCallers();
        auto callersB = funcDataB->getCallers();
        
        QITI_REQUIRE(callersA.size() == 1);
        QITI_CHECK(callersA[0] == funcDataB); // A was called by B
        QITI_CHECK(callersB.empty()); // B wasn't called by any profiled function
    }
    
    QITI_SECTION("Multiple callers")
    {
        callerTestFuncB(); // B calls A
        callerTestFuncC(); // C calls A, then C calls B (which also calls A)
        
        auto callersA = funcDataA->getCallers();
        auto callersB = funcDataB->getCallers();
        auto callersC = funcDataC->getCallers();
        
        // A should have been called by both B and C
        QITI_REQUIRE(callersA.size() == 2);
        QITI_CHECK(std::find(callersA.begin(), callersA.end(), funcDataB) != callersA.end());
        QITI_CHECK(std::find(callersA.begin(), callersA.end(), funcDataC) != callersA.end());
        
        // B should have been called by C
        QITI_REQUIRE(callersB.size() == 1);
        QITI_CHECK(callersB[0] == funcDataC);
        
        // C wasn't called by any profiled function
        QITI_CHECK(callersC.empty());
    }
}

#ifndef _WIN32 // TODO: Windows exception tracking and function type detection needs investigation
QITI_TEST_CASE("Exception tracking", FunctionDataExceptionTracking)
{
    qiti::ScopedQitiTest test;
    test.enableProfilingOnAllFunctions(true);
    
    const auto* funcDataThrows = qiti::FunctionData::getFunctionData<testFuncThrowsException>();
    const auto* funcDataCatches = qiti::FunctionData::getFunctionData<testFuncCatchesException>();
    const auto* funcDataPropagates = qiti::FunctionData::getFunctionData<testFuncThrowsUncaughtException>();
    
    QITI_SECTION("No exceptions initially")
    {
        QITI_CHECK(funcDataThrows->getNumExceptionsThrown() == 0);
        QITI_CHECK(funcDataCatches->getNumExceptionsThrown() == 0);
        QITI_CHECK(funcDataPropagates->getNumExceptionsThrown() == 0);
    }
    
    QITI_SECTION("Function that throws and catches")
    {
        testFuncCatchesException();
        
        // Only the function that actually executes throw should be marked as throwing
        QITI_CHECK(funcDataThrows->getNumExceptionsThrown() == 1);
        QITI_CHECK(funcDataCatches->getNumExceptionsThrown() == 0); // This function catches, doesn't throw
        
        // Check the call data
        auto lastCallThrows = funcDataThrows->getLastFunctionCall();
        auto lastCallCatches = funcDataCatches->getLastFunctionCall();
        
        QITI_CHECK(lastCallThrows.getNumExceptionsThrown() > 0);
        QITI_CHECK(lastCallCatches.getNumExceptionsThrown() == 0);
    }
    
    QITI_SECTION("Exception propagation")
    {
        try
        {
            testFuncThrowsUncaughtException();
        }
        catch (const std::exception&)
        {
            // Catch it here so the test doesn't terminate
        }
        
        // Only the function that actually throws should be marked as throwing
        QITI_CHECK(funcDataThrows->getNumExceptionsThrown() == 1);
        QITI_CHECK(funcDataPropagates->getNumExceptionsThrown() == 0); // This function doesn't throw, it just propagates
        
        // Check the call data
        auto lastCallThrows = funcDataThrows->getLastFunctionCall();
        auto lastCallPropagates = funcDataPropagates->getLastFunctionCall();
        
        QITI_CHECK(lastCallThrows.didThrowException() == true);
        QITI_CHECK(lastCallPropagates.didThrowException() == false); // This function doesn't throw, it just propagates
    }
    
    QITI_SECTION("Multiple exceptions")
    {
        // Call the catching function multiple times
        testFuncCatchesException();
        testFuncCatchesException();
        testFuncCatchesException();
        
        // testFuncThrowsException should have been called 3 times, each throwing
        QITI_CHECK(funcDataThrows->getNumExceptionsThrown() == 3);
        QITI_CHECK(funcDataCatches->getNumExceptionsThrown() == 0);
        QITI_CHECK(funcDataThrows->getNumTimesCalled() == 3);
        QITI_CHECK(funcDataCatches->getNumTimesCalled() == 3);
    }
}

QITI_TEST_CASE("Function type detection", FunctionDataFunctionTypeDetection)
{
    qiti::ScopedQitiTest test;
    test.enableProfilingOnAllFunctions(true);
    
    QITI_SECTION("Regular constructor")
    {
        // Create an object to trigger regular constructor
        TestClassForFunctionTypes obj1;
        
        // Get all profiled functions and find the constructor
        auto allFunctions = qiti::FunctionData::getAllProfiledFunctionData();
        const qiti::FunctionData* ctorData = nullptr;
        
        for (const auto* func : allFunctions)
        {
            if (func->isRegularConstructor())
            {
                ctorData = func;
                break;
            }
        }
        
        QITI_REQUIRE(ctorData != nullptr);
        
        QITI_CHECK(ctorData->isConstructor()        == true);
        QITI_CHECK(ctorData->isRegularConstructor() == true);
        QITI_CHECK(ctorData->isCopyConstructor()    == false);
        QITI_CHECK(ctorData->isMoveConstructor()    == false);
        QITI_CHECK(ctorData->isAssignment()         == false);
        QITI_CHECK(ctorData->isCopyAssignment()     == false);
        QITI_CHECK(ctorData->isMoveAssignment()     == false);
        QITI_CHECK(ctorData->isDestructor()         == false);
    }
    
    QITI_SECTION("Copy constructor")
    {
        TestClassForFunctionTypes obj1;
        TestClassForFunctionTypes obj2(obj1);  // Trigger copy constructor
        
        // Find the copy constructor
        auto allFunctions = qiti::FunctionData::getAllProfiledFunctionData();
        const qiti::FunctionData* copyCtorData = nullptr;
        
        for (const auto* func : allFunctions)
        {
            if (func->isCopyConstructor())
            {
                copyCtorData = func;
                break;
            }
        }
        
        QITI_REQUIRE(copyCtorData != nullptr);
        
        QITI_CHECK(copyCtorData->isConstructor()        == true);
        QITI_CHECK(copyCtorData->isRegularConstructor() == false);
        QITI_CHECK(copyCtorData->isCopyConstructor()    == true);
        QITI_CHECK(copyCtorData->isMoveConstructor()    == false);
        QITI_CHECK(copyCtorData->isAssignment()         == false);
        QITI_CHECK(copyCtorData->isCopyAssignment()     == false);
        QITI_CHECK(copyCtorData->isMoveAssignment()     == false);
        QITI_CHECK(copyCtorData->isDestructor()         == false);
    }
    
    QITI_SECTION("Move constructor")
    {
        TestClassForFunctionTypes obj1;
        TestClassForFunctionTypes obj2(std::move(obj1));  // Trigger move constructor
        
        // Find the move constructor
        auto allFunctions = qiti::FunctionData::getAllProfiledFunctionData();
        const qiti::FunctionData* moveCtorData = nullptr;
        
        for (const auto* func : allFunctions) {
            if (func->isMoveConstructor()) {
                moveCtorData = func;
                break;
            }
        }
        
        QITI_REQUIRE(moveCtorData != nullptr);
        
        QITI_CHECK(moveCtorData->isConstructor()        == true);
        QITI_CHECK(moveCtorData->isRegularConstructor() == false);
        QITI_CHECK(moveCtorData->isCopyConstructor()    == false);
        QITI_CHECK(moveCtorData->isMoveConstructor()    == true);
        QITI_CHECK(moveCtorData->isAssignment()         == false);
        QITI_CHECK(moveCtorData->isCopyAssignment()     == false);
        QITI_CHECK(moveCtorData->isMoveAssignment()     == false);
        QITI_CHECK(moveCtorData->isDestructor()         == false);
    }
    
    QITI_SECTION("Copy assignment")
    {
        TestClassForFunctionTypes obj1;
        TestClassForFunctionTypes obj2;
        obj2 = obj1;  // Trigger copy assignment
        
        // Find the copy assignment operator
        auto allFunctions = qiti::FunctionData::getAllProfiledFunctionData();
        const qiti::FunctionData* copyAssignData = nullptr;
        
        for (const auto* func : allFunctions)
        {
            if (func->isCopyAssignment())
            {
                copyAssignData = func;
                break;
            }
        }
        
        QITI_REQUIRE(copyAssignData != nullptr);
        
        QITI_CHECK(copyAssignData->isConstructor()        == false);
        QITI_CHECK(copyAssignData->isRegularConstructor() == false);
        QITI_CHECK(copyAssignData->isCopyConstructor()    == false);
        QITI_CHECK(copyAssignData->isMoveConstructor()    == false);
        QITI_CHECK(copyAssignData->isAssignment()         == true);
        QITI_CHECK(copyAssignData->isCopyAssignment()     == true);
        QITI_CHECK(copyAssignData->isMoveAssignment()     == false);
        QITI_CHECK(copyAssignData->isDestructor()         == false);
    }
    
    QITI_SECTION("Move assignment")
    {
        TestClassForFunctionTypes obj1;
        TestClassForFunctionTypes obj2;
        obj2 = std::move(obj1);  // Trigger move assignment
        
        // Find the move assignment operator
        auto allFunctions = qiti::FunctionData::getAllProfiledFunctionData();
        const qiti::FunctionData* moveAssignData = nullptr;
        
        for (const auto* func : allFunctions)
        {
            if (func->isMoveAssignment())
            {
                moveAssignData = func;
                break;
            }
        }
        
        QITI_REQUIRE(moveAssignData != nullptr);
        
        QITI_CHECK(moveAssignData->isConstructor()        == false);
        QITI_CHECK(moveAssignData->isRegularConstructor() == false);
        QITI_CHECK(moveAssignData->isCopyConstructor()    == false);
        QITI_CHECK(moveAssignData->isMoveConstructor()    == false);
        QITI_CHECK(moveAssignData->isAssignment()         == true);
        QITI_CHECK(moveAssignData->isCopyAssignment()     == false);
        QITI_CHECK(moveAssignData->isMoveAssignment()     == true);
        QITI_CHECK(moveAssignData->isDestructor()         == false);
    }
    
    QITI_SECTION("Regular method")
    {
        TestClassForFunctionTypes obj;
        obj.regularMethod();  // Call regular method
        
        // Find the regular method
        auto allFunctions = qiti::FunctionData::getAllProfiledFunctionData();
        const qiti::FunctionData* methodData = nullptr;
        
        for (const auto* func : allFunctions)
        {
            const char* name = func->getFunctionName();
            if (strstr(name, "regularMethod") != nullptr)
            {
                methodData = func;
                break;
            }
        }
        
        QITI_REQUIRE(methodData != nullptr);
        
        QITI_CHECK(methodData->isConstructor()        == false);
        QITI_CHECK(methodData->isRegularConstructor() == false);
        QITI_CHECK(methodData->isCopyConstructor()    == false);
        QITI_CHECK(methodData->isMoveConstructor()    == false);
        QITI_CHECK(methodData->isAssignment()         == false);
        QITI_CHECK(methodData->isCopyAssignment()     == false);
        QITI_CHECK(methodData->isMoveAssignment()     == false);
        QITI_CHECK(methodData->isDestructor()         == false);
    }
    
    QITI_SECTION("Destructor")
    {
        // Create an object in a nested scope to trigger destructor
        {
            TestClassForFunctionTypes obj;
        } // Destructor is called here when obj goes out of scope
        
        // Find the destructor
        auto allFunctions = qiti::FunctionData::getAllProfiledFunctionData();
        const qiti::FunctionData* destructorData = nullptr;
        
        for (const auto* func : allFunctions)
        {
            if (func->isDestructor())
            {
                destructorData = func;
                break;
            }
        }
        
        QITI_REQUIRE(destructorData != nullptr);
        
        QITI_CHECK(destructorData->isConstructor()        == false);
        QITI_CHECK(destructorData->isRegularConstructor() == false);
        QITI_CHECK(destructorData->isCopyConstructor()    == false);
        QITI_CHECK(destructorData->isMoveConstructor()    == false);
        QITI_CHECK(destructorData->isAssignment()         == false);
        QITI_CHECK(destructorData->isCopyAssignment()     == false);
        QITI_CHECK(destructorData->isMoveAssignment()     == false);
        QITI_CHECK(destructorData->isDestructor()         == true);
    }
}
#endif // ! _WIN32
