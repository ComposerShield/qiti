
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

namespace qiti::example
{
void testFunc_ThreadSanitizer0() noexcept
{
    volatile int _ = 42; // dummy code
}

void testFunc_ThreadSanitizer1() noexcept
{
    volatile int _ = 42; // dummy code
}

void incrementInThread() noexcept
{
    for (int i = 0; i < 1'000'000; ++i)
        ++counter; // Unsynchronized write
}

void TestClassThreadSanitizer::incrementInThread() noexcept
{
    for (int i = 0; i < 1'000'000; ++i)
        ++_counter; // Unsynchronized write
}

//--------------------------------------------------------------------------

void testFuncProfile() noexcept
{
    volatile int _ = 42;
}
} // namespace qiti::example
