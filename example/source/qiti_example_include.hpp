
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
// FunctionCallData
//--------------------------------------------------------------------------

int testHeapAllocationFunction() noexcept;
int testNoHeapAllocationFunction() noexcept;

//--------------------------------------------------------------------------
// profile
//--------------------------------------------------------------------------

void testFuncProfile() noexcept;

class ProfileTestType
{
    
};

//--------------------------------------------------------------------------
// ThreadSanitizer
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
} // namespace qiti::example
