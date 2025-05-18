
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
int testHeapAllocationFunction() noexcept
{
    volatile int* test = new int{0};
    delete test;
    return 42;
}

int testNoHeapAllocationFunction() noexcept
{
    volatile int test{42};
    return test;
}
} // namespace FunctionCallData

//--------------------------------------------------------------------------
namespace profile
{
void testFuncProfile() noexcept
{
    volatile int _ = 42;
}
} // namespace profile

//--------------------------------------------------------------------------

namespace ThreadSanitizer
{
void testFunc_ThreadSanitizer0() noexcept
{
    volatile int _ = 42; // dummy code
}

void testFunc_ThreadSanitizer1() noexcept
{
    volatile int _ = 42; // dummy code
}

void incrementCounter() noexcept
{
    for (int i = 0; i < 1'000'000; ++i)
        ++counter; // Unsynchronized write
}

void TestClassThreadSanitizer::incrementCounter() noexcept
{
    for (int i = 0; i < 1'000'000; ++i)
        ++_counter; // Unsynchronized write
}
} // namespace ThreadSanitizer

//--------------------------------------------------------------------------
} // namespace example
} // namespace qiti
