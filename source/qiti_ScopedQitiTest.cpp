
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_ScopedQitiTest.cpp
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

#include "qiti_FunctionData.hpp"
#include "qiti_FunctionDataUtils.hpp"
#include "qiti_MallocHooks.hpp"

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <utility>

#ifndef QITI_VERSION_MAJOR
  #warning "QITI_VERSION_MAJOR not defined, should have been set in CMakeLists.txt"
  #define QITI_VERSION_MAJOR 0
#endif

#ifndef QITI_VERSION_MINOR
  #warning "QITI_VERSION_MINOR not defined, should have been set in CMakeLists.txt"
  #define QITI_VERSION_MINOR 0
#endif

#ifndef QITI_VERSION_PATCH
  #warning "QITI_VERSION_PATCH not defined, should have been set in CMakeLists.txt"
  #define QITI_VERSION_PATCH 0
#endif

#ifndef QITI_VERSION
  #warning "QITI_VERSION not defined, should have been set in CMakeLists.txt"
  #define QITI_VERSION "<unknown>"
#endif

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

[[maybe_unused]] static void ensureInternalQitiCodeWasNotAccidentallyInstrumented() noexcept
{
    const auto allProfiledFunctions = qiti::FunctionData::getAllProfiledFunctionData();
    for (const auto* func : allProfiledFunctions)
    {
        if (func != nullptr && func->getNumTimesCalled() >= 1)
        {
            const char* funcName = func->getFunctionName();
            if (funcName != nullptr && strstr(funcName, "qiti::") != nullptr)
            {
                // Don't allow Catch2 templated functions which use qiti functions at template parameters
                if (strstr(funcName, "Catch::") != nullptr)
                    continue;
                
                // Allow qiti::example:: functions since they're meant to be profiled for testing
                if (strstr(funcName, "qiti::example::") == nullptr)
                {
                    // Found a qiti:: function that was instrumented
                    std::cout << "Internal qiti:: function was instrumented - "
                                 "missing QITI_API, QITI_API_INTERNAL or QITI_API_INLINE attribute\n";
                    std::terminate();
                }
            }
        }
    }
}
//--------------------------------------------------------------------------

ScopedQitiTest::ScopedQitiTest() noexcept
{
    // Heap allocate now before wiping memory of heap allocations
    auto newImpl = std::make_unique<Impl>();
    FunctionDataUtils::resetAll(); // start test from a blank slate
    
    [[maybe_unused]] bool qitiTestWasAlreadyRunning = qitiTestRunning.exchange(true, std::memory_order_relaxed);
    assert(! qitiTestWasAlreadyRunning); // Only one Qiti test permitted at a time
    
    impl = std::move(newImpl);
    impl->begin_time = std::chrono::steady_clock::now();
}

ScopedQitiTest::~ScopedQitiTest() noexcept
{
#if (QITI_TOPLEVEL_PROJECT==1)
    ensureInternalQitiCodeWasNotAccidentallyInstrumented();
#endif // (QITI_TOPLEVEL_PROJECT==1)
    
    [[maybe_unused]] auto ms = getLengthOfTest_ms();
    assert(ms <= impl->maxLengthOfTest_ms);
    
    qitiTestRunning.store(false, std::memory_order_relaxed);
    
    FunctionDataUtils::resetAll(); // clean up after ourselves
}

void ScopedQitiTest::reset(bool resetTestStartTime) noexcept
{
    FunctionDataUtils::resetAll();
    
    if (resetTestStartTime)
        impl->begin_time = std::chrono::steady_clock::now();
}

void ScopedQitiTest::enableProfilingOnAllFunctions(bool enable) noexcept
{
    enable ? Profile::beginProfilingAllFunctions()
           : Profile::endProfilingAllFunctions();
}

const char* ScopedQitiTest::getQitiVersionString() noexcept
{
    static constexpr const char* version = QITI_VERSION; // set in CMakeLists.txt or qiti_API.hpp
    return version;
}

int ScopedQitiTest::getQitiVersionMajor() noexcept
{
    static constexpr int version = QITI_VERSION_MAJOR;   // set in CMakeLists.txt or qiti_API.hpp
    return version;
}

int ScopedQitiTest::getQitiVersionMinor() noexcept
{
    static constexpr int version = QITI_VERSION_MINOR;   // set in CMakeLists.txt or qiti_API.hpp
    return version;
}

int ScopedQitiTest::getQitiVersionPatch() noexcept
{
    static constexpr int version = QITI_VERSION_PATCH;   // set in CMakeLists.txt or qiti_API.hpp
    return version;
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
