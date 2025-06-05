
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

#include "qiti_profile.hpp"

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
namespace profile
{
void resetProfiling() noexcept
{        
    g_functionsToProfile.clear();
    g_profileAllFunctions = false;
    MallocHooks::numHeapAllocationsOnCurrentThread  = 0u;
    MallocHooks::amountHeapAllocatedOnCurrentThread = 0ull;
}

void beginProfilingFunction(const void* functionAddress, const char* functionName) noexcept
{
    g_functionsToProfile.insert(functionAddress);
    
    // This adds the function to our function map
    (void)qiti::getFunctionDataFromAddress(functionAddress, functionName);
}

void endProfilingFunction(const void* functionAddress) noexcept
{
    g_functionsToProfile.erase(functionAddress);
}

void beginProfilingAllFunctions() noexcept
{
    g_profileAllFunctions = true;
}

void endProfilingAllFunctions() noexcept
{
    g_profileAllFunctions = false;
}

bool isProfilingFunction(const void* funcAddress) noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    return g_profileAllFunctions || g_functionsToProfile.contains(funcAddress);
}

void beginProfilingType(std::type_index /*functionAddress*/) noexcept
{
    
}

void endProfilingType(std::type_index /*functionAddress*/) noexcept
{
    
}

uint64_t getNumHeapAllocationsOnCurrentThread() noexcept
{
    return MallocHooks::numHeapAllocationsOnCurrentThread;
}

uint64_t getAmountHeapAllocatedOnCurrentThread() noexcept
{
    return MallocHooks::amountHeapAllocatedOnCurrentThread;
}

void updateFunctionDataOnEnter(const void* this_fn) noexcept
{
    // Update FunctionData
    auto& functionData = qiti::getFunctionDataFromAddress(this_fn);
    
    qiti::ScopedNoHeapAllocations noAlloc; // TODO: can we move this up to very top?
    
    auto* impl = functionData.getImpl();
    functionData.functionCalled();
    
    for (auto* listener : impl->listeners)
        listener->onFunctionEnter(&functionData);
    
    // Update FunctionCallData
    impl->lastCallData.reset(); // Deletes previous impl
    
    auto* lastCallImpl = impl->lastCallData.getImpl();
    lastCallImpl->begin_time = std::chrono::steady_clock::now();
    lastCallImpl->callingThread = std::this_thread::get_id();
    lastCallImpl->numHeapAllocationsBeforeFunctionCall = MallocHooks::numHeapAllocationsOnCurrentThread;
    lastCallImpl->amountHeapAllocatedBeforeFunctionCall = MallocHooks::amountHeapAllocatedOnCurrentThread;
}

void updateFunctionDataOnExit(const void* this_fn) noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    auto& functionData = qiti::getFunctionDataFromAddress(this_fn);
    auto* impl = functionData.getImpl();
    auto* callImpl = impl->lastCallData.getImpl();
    
    const auto end_time = std::chrono::steady_clock::now();
    const auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - callImpl->begin_time);
    
    for (auto* listener : impl->listeners)
        listener->onFunctionExit(&functionData);

    // Update FunctionCallData
    callImpl->end_time = end_time;
    callImpl->timeSpentInFunctionNanoseconds = static_cast<uint64_t>(elapsed_ns.count());
    callImpl->numHeapAllocationsAfterFunctionCall = MallocHooks::numHeapAllocationsOnCurrentThread;
    callImpl->amountHeapAllocatedAfterFunctionCall = MallocHooks::amountHeapAllocatedOnCurrentThread;
    
    // Update average time spent in function (must be after FunctionCallData is finished)
    if(impl->averageTimeSpentInFunctionNanoseconds == 0)
        impl->averageTimeSpentInFunctionNanoseconds = callImpl->timeSpentInFunctionNanoseconds;
    else
    {
        const auto currentAverage = impl->averageTimeSpentInFunctionNanoseconds;
        auto effectiveTotal = currentAverage * (impl->numTimesCalled-1);
        effectiveTotal += callImpl->timeSpentInFunctionNanoseconds;
        const auto newAverage = (impl->numTimesCalled > 0) // prevent divide by zero
                                ? (effectiveTotal / impl->numTimesCalled)
                                : 0;
        impl->averageTimeSpentInFunctionNanoseconds = newAverage;
    }
}
} // namespace profile
} // namespace qiti
