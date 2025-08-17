
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

#include <atomic>

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
double fastWork() noexcept;
double slowWork() noexcept;
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
    void incrementCounter(std::atomic<int>& ready, std::atomic<bool>& go) noexcept;

private:
    volatile int _counter = 0;
};
} // namespace ThreadSanitizer

//--------------------------------------------------------------------------

namespace utils
{
void testFunc0() noexcept;
} // namespace utils

//--------------------------------------------------------------------------

namespace HotspotDetector
{
void hotspotTestFuncSlow() noexcept;
void hotspotTestFuncFast() noexcept;
void hotspotTestFuncThrows();
void hotspotTestFuncCatches();
} // namespace HotspotDetector

//--------------------------------------------------------------------------
} // namespace example
} // namespace qiti
