
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_profile.cpp
 *
 * @author   Adam Shield
 * @date     2025-05-16
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "qiti_Profile.hpp"

#include "qiti_FunctionCallData_Impl.hpp"
#include "qiti_FunctionCallData.hpp"
#include "qiti_FunctionData_Impl.hpp"
#include "qiti_FunctionData.hpp"
#include "qiti_MallocHooks.hpp"
#include "qiti_ScopedNoHeapAllocations.hpp"

#include <execinfo.h>
#include <cxxabi.h>
#include <unistd.h>

#include <cassert>
#include <iostream>
#include <memory>
#include <mutex>
#include <regex>
#include <unordered_set>
#include <utility>
#include <string>

//--------------------------------------------------------------------------

inline std::unordered_set<const void*> g_functionsToProfile;
bool g_profileAllFunctions = false;

struct Init_g_functionsToProfile
{
    Init_g_functionsToProfile()
    {
        g_functionsToProfile.reserve(256); // automatically reserve on startup
    }
};
static const Init_g_functionsToProfile init_g_functionsToProfile;

//--------------------------------------------------------------------------

namespace qiti
{
void Profile::resetProfiling() noexcept
{        
    g_functionsToProfile.clear();
    g_profileAllFunctions = false;
    MallocHooks::numHeapAllocationsOnCurrentThread  = 0u;
    MallocHooks::amountHeapAllocatedOnCurrentThread = 0ull;
}

void Profile::beginProfilingFunction(const void* functionAddress, const char* functionName) noexcept
{
    g_functionsToProfile.insert(functionAddress);
    
    // This adds the function to our function map
    (void)Utils::getFunctionDataFromAddress(functionAddress, functionName);
}

void Profile::endProfilingFunction(const void* functionAddress) noexcept
{
    g_functionsToProfile.erase(functionAddress);
}

void Profile::beginProfilingAllFunctions() noexcept
{
    g_profileAllFunctions = true;
}

void Profile::endProfilingAllFunctions() noexcept
{
    g_profileAllFunctions = false;
}

bool Profile::isProfilingFunction(const void* funcAddress) noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    return g_profileAllFunctions || g_functionsToProfile.contains(funcAddress);
}

void Profile::beginProfilingType(std::type_index /*functionAddress*/) noexcept
{
    
}

void Profile::endProfilingType(std::type_index /*functionAddress*/) noexcept
{
    
}

uint64_t Profile::getNumHeapAllocationsOnCurrentThread() noexcept
{
    return MallocHooks::numHeapAllocationsOnCurrentThread;
}

uint64_t Profile::getAmountHeapAllocatedOnCurrentThread() noexcept
{
    return MallocHooks::amountHeapAllocatedOnCurrentThread;
}

void Profile::updateFunctionDataOnEnter(const void* this_fn) noexcept
{
    // Update FunctionData
    auto& functionData = Utils::getFunctionDataFromAddress(this_fn);
    
    qiti::ScopedNoHeapAllocations noAlloc; // TODO: can we move this up to very top?
    
    auto* impl = functionData.getImpl();
    functionData.functionCalled();
    
    for (auto* listener : impl->listeners)
        listener->onFunctionEnter(&functionData);
    
    // Update FunctionCallData
    impl->lastCallData.reset(); // Deletes previous impl
    
    auto* lastCallImpl = impl->lastCallData.getImpl();
    lastCallImpl->callingThread = std::this_thread::get_id();
    lastCallImpl->numHeapAllocationsBeforeFunctionCall = MallocHooks::numHeapAllocationsOnCurrentThread;
    lastCallImpl->amountHeapAllocatedBeforeFunctionCall = MallocHooks::amountHeapAllocatedOnCurrentThread;
    
    // Grab starting times last without doing additional work after
    lastCallImpl->startTimeWallClock = std::chrono::steady_clock::now();
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &lastCallImpl->startTimeCpu); // last to be most precise
}

void Profile::updateFunctionDataOnExit(const void* this_fn) noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    auto& functionData = Utils::getFunctionDataFromAddress(this_fn);
    auto* impl = functionData.getImpl();
    auto* callImpl = impl->lastCallData.getImpl();
    timespec cpuEndTime;
    
    // Get end times immediately before doing any other work
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &cpuEndTime); // first to be most precise
    const auto clockEndTime = std::chrono::steady_clock::now();
    
    // Get elapsed times
    const auto cpuElapsed_ns =
        (static_cast<uint64_t>(cpuEndTime.tv_sec - callImpl->startTimeCpu.tv_sec) * 1'000'000'000ULL) +
        (static_cast<uint64_t>(cpuEndTime.tv_nsec) - static_cast<uint64_t>(callImpl->startTimeCpu.tv_nsec));
    const auto clockElapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(clockEndTime - callImpl->startTimeWallClock);

    // Update FunctionCallData (before updating listeners in case listeners need that information)
    callImpl->endTimeWallClock = clockEndTime;
    callImpl->endTimeCpu = cpuEndTime;
    callImpl->timeSpentInFunctionNanosecondsWallClock = static_cast<uint64_t>(clockElapsed_ns.count());
    callImpl->timeSpentInFunctionNanosecondsCpu = cpuElapsed_ns;
    callImpl->numHeapAllocationsAfterFunctionCall = MallocHooks::numHeapAllocationsOnCurrentThread;
    callImpl->amountHeapAllocatedAfterFunctionCall = MallocHooks::amountHeapAllocatedOnCurrentThread;
    
    // Update listeners
    for (auto* listener : impl->listeners)
        listener->onFunctionExit(&functionData);
    
    // Update average time spent in function (must be after FunctionCallData is finished)
    if(impl->averageTimeSpentInFunctionNanosecondsWallClock == 0)
        impl->averageTimeSpentInFunctionNanosecondsWallClock = callImpl->timeSpentInFunctionNanosecondsWallClock;
    else
    {
        const auto currentAverage = impl->averageTimeSpentInFunctionNanosecondsWallClock;
        auto effectiveTotal = currentAverage * (impl->numTimesCalled-1);
        effectiveTotal += callImpl->timeSpentInFunctionNanosecondsWallClock;
        const auto newAverage = (impl->numTimesCalled > 0) // prevent divide by zero
                                ? (effectiveTotal / impl->numTimesCalled)
                                : 0;
        impl->averageTimeSpentInFunctionNanosecondsWallClock = newAverage;
    }
    
    if(impl->averageTimeSpentInFunctionNanosecondsCpu == 0)
        impl->averageTimeSpentInFunctionNanosecondsCpu = callImpl->timeSpentInFunctionNanosecondsCpu;
    else
    {
        const auto currentAverage = impl->averageTimeSpentInFunctionNanosecondsCpu;
        auto effectiveTotal = currentAverage * (impl->numTimesCalled-1);
        effectiveTotal += callImpl->timeSpentInFunctionNanosecondsCpu;
        const auto newAverage = (impl->numTimesCalled > 0) // prevent divide by zero
                                ? (effectiveTotal / impl->numTimesCalled)
                                : 0;
        impl->averageTimeSpentInFunctionNanosecondsCpu = newAverage;
    }
}
} // namespace qiti
