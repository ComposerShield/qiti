
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

namespace qiti
{
namespace example
{
//--------------------------------------------------------------------------

namespace FunctionCallData
{
int testHeapAllocation() noexcept;
int testNoHeapAllocation() noexcept;
} // namespace FunctionCallData

//--------------------------------------------------------------------------

namespace profile
{
void testFunc() noexcept;

class ProfileTestType
{
    
};
} // namespace profile

//--------------------------------------------------------------------------

namespace ThreadSanitizer
{
void testFunc0() noexcept;
void testFunc1() noexcept;

void incrementCounter() noexcept;

class TestClass
{
public:
    void incrementCounter() noexcept;
private:
    int _counter = 0;
};
} // namespace ThreadSanitizer

//--------------------------------------------------------------------------
} // namespace example
} // namespace qiti
