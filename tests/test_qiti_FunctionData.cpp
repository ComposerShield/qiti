
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"
// Basic Catch2 macros
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <random>
#include <thread>
#include <vector>

#include <iostream>

//--------------------------------------------------------------------------

/** NOT static to purposely allow external linkage and visibility to QITI */
__attribute__((noinline))
__attribute__((optnone))
void testFunc() noexcept
{
    volatile int _ = 42;
}

//--------------------------------------------------------------------------

TEST_CASE("qiti::FunctionData::getFunctionName()")
{
    qiti::ScopedQitiTest test;

    {
        qiti::Profile::beginProfilingFunction<&testFunc>();
        auto functionData = qiti::Utils::getFunctionData<&testFunc>();
        std::string name = functionData->getFunctionName();
        QITI_CHECK(name == "testFunc");
    }
}

TEST_CASE("qiti::FunctionData::getNumTimesCalled()")
{
    qiti::ScopedQitiTest test;
    
    qiti::Profile::beginProfilingFunction<&testFunc>();
    
    auto funcData = qiti::Utils::getFunctionData<&testFunc>();
    QITI_REQUIRE(funcData != nullptr);
    
    SECTION("Called twice")
    {
        testFunc();
        testFunc();
        QITI_CHECK(funcData->getNumTimesCalled() == 2);
    }
    
    SECTION("Not called")
    {
        QITI_CHECK(funcData->getNumTimesCalled() == 0);
    }
    
    SECTION("Called twice on two different threads")
    {
        std::thread t([]{ testFunc(); });
        testFunc();
        t.join();
        QITI_CHECK(funcData->getNumTimesCalled() == 2);
    }
}

TEST_CASE("qiti::FunctionData::getNumTimesCalled(), using static constructor")
{
    qiti::ScopedQitiTest test;
    
    auto funcData = qiti::FunctionData::getFunctionData<&testFunc>();
    QITI_REQUIRE(funcData != nullptr);
    
    SECTION("Called once")
    {
        testFunc();
        QITI_CHECK(funcData->getNumTimesCalled() == 1);
    }
    
    SECTION("Not called")
    {
        QITI_CHECK(funcData->getNumTimesCalled() == 0);
    }
}

TEST_CASE("qiti::FunctionData::wasCalledOnThread()")
{
    qiti::ScopedQitiTest test;
    
    qiti::Profile::beginProfilingFunction<&testFunc>();
    
    auto funcData = qiti::Utils::getFunctionData<&testFunc>();
    QITI_REQUIRE(funcData != nullptr);
    
    SECTION("Function called on current thread")
    {
        testFunc();
        
        std::thread::id currentThread = std::this_thread::get_id();
        
        QITI_CHECK(funcData->wasCalledOnThread(currentThread));
    }
    
    SECTION("Function never called")
    {
        std::thread::id currentThread = std::this_thread::get_id();
        
        QITI_CHECK(! funcData->wasCalledOnThread(currentThread));
    }
    
    SECTION("Function called on custom thread")
    {
        std::thread thread([]{ testFunc(); });
        auto id = thread.get_id();
        thread.join(); // sync with thread to ensure function was called
        
        QITI_CHECK(funcData->wasCalledOnThread(id));
    }
}

TEST_CASE("qiti::FunctionData::getAllProfiledFunctionData()")
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
