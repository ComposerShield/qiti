
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_example.cpp
 *
 * @author   Adam Shield
 * @date     2025-05-18
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "qiti_example_include.hpp"

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
  #include <immintrin.h>  // for _mm_pause()
#endif

#include <cmath>
#include <thread>

// Disable optimizations for this entire file to prevent Release mode optimizations
// from interfering with timing-sensitive thread synchronization and race conditions
// that unit tests rely on to function correctly
#pragma clang optimize off

//--------------------------------------------------------------------------

int counter = 0; // Shared global variable

//--------------------------------------------------------------------------

namespace qiti
{
namespace example
{
//--------------------------------------------------------------------------
namespace FunctionCallData
{
int testHeapAllocation() noexcept
{
    volatile int* test = new int{0};
    delete test;
    return 42;
}

int testNoHeapAllocation() noexcept
{
    volatile int test{42};
    return test;
}

double fastWork() noexcept
{
    volatile double result = 1.0;
    for (int i = 0; i < 10000; ++i) {
        result += i * 0.001;
    }
    return result;
}

double slowWork() noexcept
{
    volatile double result = 1.0;
    for (int i = 0; i < 100000; ++i) {
        result += i * 0.001;
    }
    return result;
}
} // namespace FunctionCallData

//--------------------------------------------------------------------------
namespace profile
{
void testFunc() noexcept
{
    volatile int _ = 42;
}

int testHeapAllocation() noexcept
{
    volatile int* test = new int{0};
    delete test;
    return 42;
}

int TestType::testFunc() const noexcept
{
    return 0;
}
} // namespace profile

//--------------------------------------------------------------------------

namespace ThreadSanitizer
{
void testFunc0() noexcept
{
    volatile int _ = 42; // dummy code
}

void testFunc1() noexcept
{
    volatile int _ = 42; // dummy code
}

void incrementCounter() noexcept
{
    volatile int dummyInternalVal = 0;
    for (int i = 0; i < 1'000'000; ++i)
    {
        dummyInternalVal += 1; // prevent re-ordering
        ++counter;             // Unsynchronized write
    }
}

void TestClass::incrementCounter(std::atomic<int>& ready, std::atomic<bool>& go) noexcept
{
    ready.fetch_add(1, std::memory_order_relaxed);
    while (! go.load(std::memory_order_acquire)) { /* spin */ }

    for (int i = 0; i < 5'000'000; ++i)
    {
        _counter = counter + 1; // read and write
        (void)_counter;         // read
    }
}
} // namespace ThreadSanitizer

//--------------------------------------------------------------------------

namespace utils
{
void testFunc0() noexcept
{
    volatile int _ = 42;
}
} // namespace utils

//--------------------------------------------------------------------------
} // namespace example
} // namespace qiti

// Re-enable optimizations for subsequent files
#pragma clang optimize on
