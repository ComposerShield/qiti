
#include <qiti_include.hpp>

#include "qiti_test_macros.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <random>
#include <thread>
#include <vector>

#include <iostream>

/** NOT static to purposely allow external linkage and visibility to QITI */
__attribute__((noinline)) __attribute__((optnone))
void testFunc() noexcept
{
    std::cout << "testFunc start\n";
    volatile int _ = 42;
    std::cout << "testFunc end\n";
}

TEST_CASE("qiti::FunctionData::getFunctionName()")
{
    qiti::resetAll();
    
    qiti::profile::beginProfilingFunction<&testFunc>();
    auto functionData = qiti::getFunctionData<&testFunc>();
    std::string name = functionData->getFunctionName();
    QITI_CHECK(name == "testFunc()");

    qiti::resetAll();
}

TEST_CASE("qiti::FunctionData::getNumTimesCalled()")
{
    qiti::resetAll();
    
    qiti::profile::beginProfilingFunction<&testFunc>();
    
    SECTION("Called twice")
    {
        testFunc();
        testFunc();
        
        auto funcData = qiti::getFunctionData<&testFunc>();
        QITI_REQUIRE(funcData != nullptr);
        
        QITI_CHECK(funcData->getNumTimesCalled() == 2);
    }
    
    SECTION("Not called")
    {
        auto funcData = qiti::getFunctionData<&testFunc>();
        QITI_REQUIRE(funcData != nullptr);
        
        QITI_CHECK(funcData->getNumTimesCalled() == 0);
    }
    
    qiti::resetAll();
}

TEST_CASE("qiti::FunctionData::wasCalledOnThread()")
{
    qiti::resetAll();
    
    qiti::profile::beginProfilingFunction<&testFunc>();
    
    auto funcData = qiti::getFunctionData<&testFunc>();
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

    qiti::resetAll();
}
