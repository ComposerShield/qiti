
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_example_include.hpp
 *
 * @author   Adam Shield
 * @date     2025-05-18
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#pragma once

//--------------------------------------------------------------------------

namespace qiti::example
{
//--------------------------------------------------------------------------
// Thread Sanitizer
//--------------------------------------------------------------------------

void testFunc_ThreadSanitizer0() noexcept;
void testFunc_ThreadSanitizer1() noexcept;

void incrementInThread() noexcept;

class TestClassThreadSanitizer
{
public:
    void incrementInThread() noexcept;
private:
    int _counter = 0;
};

//--------------------------------------------------------------------------
// Profiling
//--------------------------------------------------------------------------

void testFuncProfile() noexcept;

class ProfileTestType
{
    
};
} // namespace qiti::example
