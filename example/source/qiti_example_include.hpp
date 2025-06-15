
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
double someWork() noexcept;
double moreWork() noexcept;
} // namespace FunctionCallData

//--------------------------------------------------------------------------

namespace profile
{
void testFunc() noexcept;
int testHeapAllocation() noexcept;

class TestType
{
public:
    int testFunc() const noexcept;
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

namespace utils
{
void testFunc0() noexcept;
} // namespace utils

//--------------------------------------------------------------------------
} // namespace example
} // namespace qiti
