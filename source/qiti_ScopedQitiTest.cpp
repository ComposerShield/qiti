
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

#include "qiti_MallocHooks.hpp"
#include "qiti_Utils.hpp"

#include <atomic>
#include <cassert>
#include <chrono>
#include <limits>
#include <memory>
#include <utility>

//--------------------------------------------------------------------------
static std::atomic<bool> qitiTestRunning = false;

bool QITI_API_INTERNAL isQitiTestRunning() noexcept
{
    return qitiTestRunning.load(std::memory_order_relaxed);
}
//--------------------------------------------------------------------------
namespace qiti
{
//--------------------------------------------------------------------------
struct ScopedQitiTest::Impl
{
    std::chrono::steady_clock::time_point begin_time;
    
    uint64_t maxLengthOfTest_ms = std::numeric_limits<uint64_t>::max();
};
//--------------------------------------------------------------------------

ScopedQitiTest::ScopedQitiTest() noexcept
{
    // Heap allocate now before wiping memory of heap allocations
    auto newImpl = std::make_unique<Impl>();
    Utils::resetAll(); // start test from a blank slate
    
    bool qitiTestWasAlreadyRunning = qitiTestRunning.exchange(true, std::memory_order_relaxed);
    assert(! qitiTestWasAlreadyRunning); // Only one Qiti test permitted at a time
    
    impl = std::move(newImpl);
    impl->begin_time = std::chrono::steady_clock::now();
}

ScopedQitiTest::~ScopedQitiTest() noexcept
{
    auto ms = getLengthOfTest_ms();
    assert(ms <= impl->maxLengthOfTest_ms);
    
    qitiTestRunning.store(false, std::memory_order_relaxed);
    
    Utils::resetAll(); // clean up after ourselves
}

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

void ScopedQitiTest::setMaximumDurationOfTest_ms(uint64_t ms) noexcept
{
    impl->maxLengthOfTest_ms = ms;
}

//--------------------------------------------------------------------------
} // namespace qiti
//--------------------------------------------------------------------------
