
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

#include <thread>

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

// Despite egregiously being a data race, CI does not always detect it as a data race.
// So we added some waits to make sure our tests don't intermittently fail

__attribute__((noinline))
__attribute__((optnone))
void TestClass::incrementCounter() noexcept
{
    volatile int dummyInternalVal = 0;
    for (int i = 0; i < 1'000'000; ++i)
    {
        dummyInternalVal += 1;   // prevent re-ordering
        ++_counter;              // Unsynchronized write
        if (_counter % 2 == 0)   // Unsynchronized read
            std::this_thread::yield(); // hint to scheduler, very short pause
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
