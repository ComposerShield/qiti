
#include "qiti_include.hpp"

#include "qiti_ThreadSanitizer.hpp"

#include "qiti_test_macros.hpp"

#include <catch2/catch_test_macros.hpp>

/** NOT static to purposely allow external linkage and visibility to QITI */
__attribute__((noinline)) __attribute__((optnone))
void testFunc_ThreadSanitizer0() noexcept
{
    volatile int _ = 42;
}

/** NOT static to purposely allow external linkage and visibility to QITI */
__attribute__((noinline)) __attribute__((optnone))
void testFunc_ThreadSanitizer1() noexcept
{
    volatile int _ = 42;
}

TEST_CASE("qiti::ThreadSanitizer::functionsNotCalledInParallel")
{
    qiti::resetAll();
    
    auto tsan = qiti::ThreadSanitizer::functionsNotCalledInParallel<testFunc_ThreadSanitizer0, testFunc_ThreadSanitizer1>();
    
    QITI_REQUIRE(tsan.passed());

    qiti::resetAll();
}
