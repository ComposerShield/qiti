
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_ScopedNoHeapAllocations.hpp
 *
 * @author   Adam Shield
 * @date     2025-05-16
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "qiti_ScopedQitiTest.hpp"

#include "qiti_utils.hpp"

#include <chrono>

//--------------------------------------------------------------------------
namespace qiti
{
//--------------------------------------------------------------------------
struct ScopedQitiTest::Impl
{
    std::chrono::steady_clock::time_point begin_time;
};
//--------------------------------------------------------------------------

ScopedQitiTest::ScopedQitiTest() noexcept
{
    qiti::resetAll(); // start test from a blank slate
    impl = std::make_unique<Impl>();
    impl->begin_time = std::chrono::steady_clock::now();
}

ScopedQitiTest::~ScopedQitiTest() noexcept = default;

uint64_t ScopedQitiTest::getLengthOfTest_ms() const noexcept
{
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_ns = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - impl->begin_time);
    return static_cast<uint64_t>(elapsed_ns.count());
}

uint64_t ScopedQitiTest::getLengthOfTest_ns() const noexcept
{
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - impl->begin_time);
    return static_cast<uint64_t>(elapsed_ns.count());
}

//--------------------------------------------------------------------------
} // namespace qiti
//--------------------------------------------------------------------------
