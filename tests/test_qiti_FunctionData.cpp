
#include <qiti_include.hpp>

#include "qiti_test_macros.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <vector>
#include <random>

/** NOT static to purposely allow external linkage and visibility to QITI */
__attribute__((noinline)) __attribute__((optnone))
void testFunc()
{
    volatile int _ = 42;
}

TEST_CASE("qiti::FunctionData::getFunctionName()")
{
    qiti::resetAll();

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
