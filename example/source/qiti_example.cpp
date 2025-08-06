
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

__attribute__((optnone))
__attribute__((noinline))
inline static void cpu_pause() noexcept
{
    // x86 or x86_64:
    #if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
        _mm_pause();
    
    // AArch64 (Apple Silicon, many ARM64 Linux ports, etc.):
    #elif defined(__aarch64__) || defined(_M_ARM64) || defined(__arm64__)
        __asm__ volatile("yield");
    
    // Fallback (any other architecture): just yield to scheduler
    #else
        std::this_thread::yield(); // hint to scheduler, very short pause
    #endif
}

//--------------------------------------------------------------------------

namespace qiti
{
namespace example
{
//--------------------------------------------------------------------------
namespace FunctionCallData
{
__attribute__((optnone))
__attribute__((noinline))
int testHeapAllocation() noexcept
{
    volatile int* test = new int{0};
    delete test;
    return 42;
}

__attribute__((optnone))
__attribute__((noinline))
int testNoHeapAllocation() noexcept
{
    volatile int test{42};
    return test;
}

__attribute__((noinline))
double fastWork() noexcept
{
    volatile double result = 1.0;
    for (int i = 0; i < 10000; ++i) {
        result += i * 0.001;
    }
    return result;
}

__attribute__((noinline))
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
__attribute__((optnone))
__attribute__((noinline))
void testFunc() noexcept
{
    volatile int _ = 42;
}

__attribute__((optnone))
__attribute__((noinline))
int testHeapAllocation() noexcept
{
    volatile int* test = new int{0};
    delete test;
    return 42;
}

__attribute__((optnone))
__attribute__((noinline))
int TestType::testFunc() const noexcept
{
    return 0;
}
} // namespace profile

//--------------------------------------------------------------------------

namespace ThreadSanitizer
{
__attribute__((optnone))
__attribute__((noinline))
void testFunc0() noexcept
{
    volatile int _ = 42; // dummy code
}

__attribute__((optnone))
__attribute__((noinline))
void testFunc1() noexcept
{
    volatile int _ = 42; // dummy code
}

__attribute__((optnone))
__attribute__((noinline))
void incrementCounter() noexcept
{
    volatile int dummyInternalVal = 0;
    for (int i = 0; i < 1'000'000; ++i)
    {
        dummyInternalVal += 1; // prevent re-ordering
        ++counter;             // Unsynchronized write
    }
}

// Despite egregiously being a data race, CI does not always detect it as a data race.
// So we added some waits to make sure our tests don't intermittently fail
__attribute__((optnone))
__attribute__((noinline))
void TestClass::incrementCounter() noexcept
{
    volatile int dummyInternalVal = 0;
    for (int i = 0; i < 1'000'000; ++i)
    {
        dummyInternalVal += 1;   // prevent re-ordering
        ++_counter;              // Unsynchronized write
        if (_counter % 2 == 0)   // Unsynchronized read
            // spin for a brief moment, forcing both threads to sit in this
            // branch at the same time more often
            for (int k = 0; k < 100; ++k)
                cpu_pause();
    }
}
} // namespace ThreadSanitizer

//--------------------------------------------------------------------------

namespace utils
{
__attribute__((optnone))
__attribute__((noinline))
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
