
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

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#else
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <unistd.h>
#endif

#include <cassert>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <mutex>
#include <regex>
#include <stack>
#include <unordered_set>
#include <utility>
#include <string>

#ifdef _WIN32
QITI_API_INTERNAL static void clock_gettime_windows(timespec& time) noexcept
{
    // Windows: Try GetThreadTimes first for CPU time
    FILETIME creationTime, exitTime, kernelTime, userTime;
    if (GetThreadTimes(GetCurrentThread(), &creationTime, &exitTime, &kernelTime, &userTime))
    {
        ULARGE_INTEGER userTimeLI, kernelTimeLI;
        userTimeLI.LowPart = userTime.dwLowDateTime;
        userTimeLI.HighPart = userTime.dwHighDateTime;
        kernelTimeLI.LowPart = kernelTime.dwLowDateTime;
        kernelTimeLI.HighPart = kernelTime.dwHighDateTime;
        
        uint64_t totalTime100ns = userTimeLI.QuadPart + kernelTimeLI.QuadPart;
        time.tv_sec = static_cast<time_t>(totalTime100ns / 10000000ULL);
        time.tv_nsec = static_cast<long>((totalTime100ns % 10000000ULL) * 100ULL); // NOLINT
    }
    else
    {
        // Fallback: Use QueryPerformanceCounter for high-resolution timing
        static LARGE_INTEGER frequency = {{0, 0}};
        if (frequency.QuadPart == 0)
        {
            QueryPerformanceFrequency(&frequency);
        }
        
        LARGE_INTEGER counter;
        if (QueryPerformanceCounter(&counter) && frequency.QuadPart > 0)
        {
            // Convert to nanoseconds - use static_cast to handle sign conversion
            uint64_t counterValue = static_cast<uint64_t>(counter.QuadPart);
            uint64_t frequencyValue = static_cast<uint64_t>(frequency.QuadPart);
            uint64_t totalNs = (counterValue * 1000000000ULL) / frequencyValue;
            time.tv_sec = static_cast<time_t>(totalNs / 1000000000ULL);
            time.tv_nsec = static_cast<long>(totalNs % 1000000000ULL); // NOLINT
        }
        else
        {
            // Last resort: use current time (not ideal for CPU time, but better than zero)
            FILETIME ft;
            GetSystemTimeAsFileTime(&ft);
            ULARGE_INTEGER li;
            li.LowPart = ft.dwLowDateTime;
            li.HighPart = ft.dwHighDateTime;
            
            // Convert from 100ns intervals since Jan 1, 1601 to seconds/nanoseconds
            uint64_t time100ns = li.QuadPart;
            time.tv_sec = static_cast<time_t>(time100ns / 10000000ULL);
            time.tv_nsec = static_cast<long>((time100ns % 10000000ULL) * 100ULL); // NOLINT
        }
    }
}
#endif

namespace qiti
{
//--------------------------------------------------------------------------

inline std::unordered_set<const void*> g_functionsToProfile;
bool g_profileAllFunctions = false;

// Thread-local call stack to track caller relationships
thread_local std::stack<qiti::FunctionData*> g_callStack;

static thread_local bool g_profilingEnabled = true;

#ifndef _WIN32
struct Init_g_functionsToProfile
{
    Init_g_functionsToProfile()
    {
        g_functionsToProfile.reserve(256); // automatically reserve on startup
    }
};
static const Init_g_functionsToProfile init_g_functionsToProfile;
#endif

//--------------------------------------------------------------------------

Profile::ScopedDisableProfiling::ScopedDisableProfiling() noexcept
: wasProfilingEnabled(g_profilingEnabled)
{
    g_profilingEnabled = false;
}

Profile::ScopedDisableProfiling::~ScopedDisableProfiling() noexcept
{
    g_profilingEnabled = wasProfilingEnabled;
}

//--------------------------------------------------------------------------

void Profile::resetProfiling() noexcept
{        
    g_functionsToProfile.clear();
    g_profileAllFunctions = false;
    qiti::MallocHooks::getNumHeapAllocationsOnCurrentThread() = 0u;
    qiti::MallocHooks::getTotalAmountHeapAllocatedOnCurrentThread() = 0ull;
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
    
    // mechanism to disable qiti profiling within qiti functions
    if (! g_profilingEnabled)
        return false;
    
    if (g_functionsToProfile.contains(funcAddress))
        return true;
    
    if (! g_profileAllFunctions)
        return false;
    
    // When profiling all functions, skip templated functions that use qiti types as
    // template parameters (e.g. std::vector<qiti::FunctionData*>)
    Dl_info info;
    if (dladdr(funcAddress, &info) && info.dli_sname)
    {
#ifdef _WIN32
        // Windows: Use UnDecorateSymbolName for demangling
        char buffer[1024];
        if (UnDecorateSymbolName(info.dli_sname, buffer, sizeof(buffer), UNDNAME_COMPLETE))
        {
            bool shouldSkip = strstr(buffer, "<qiti::") != nullptr;
            if (shouldSkip)
                return false;
        }
#else
        int status = 0;
        char* demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
        
        if (status == 0 && demangled)
        {
            bool shouldSkip = strstr(demangled, "<qiti::") != nullptr;
            free(demangled);
            if (shouldSkip)
                return false;
        }
#endif
    }
    
    return true;
}

void Profile::beginProfilingType(std::type_index /*functionAddress*/) noexcept
{
    
}

void Profile::endProfilingType(std::type_index /*functionAddress*/) noexcept
{
    
}

uint64_t Profile::getNumHeapAllocationsOnCurrentThread() noexcept
{
    return qiti::MallocHooks::getNumHeapAllocationsOnCurrentThread();
}

uint64_t Profile::getAmountHeapAllocatedOnCurrentThread() noexcept
{
    return qiti::MallocHooks::getTotalAmountHeapAllocatedOnCurrentThread();
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
    
    // Track caller relationship - check if there's a caller on the stack
    const FunctionData* caller = nullptr;
    if (! g_callStack.empty())
    {
        caller = g_callStack.top();
        if (caller != nullptr)
            impl->callers.insert(caller);
    }
    
    // Push this function onto the call stack
    g_callStack.push(&functionData);
    
    auto* lastCallImpl = impl->lastCallData.getImpl();
    lastCallImpl->caller = caller;
    lastCallImpl->callingThread = std::this_thread::get_id();
    lastCallImpl->numHeapAllocationsBeforeFunctionCall = qiti::MallocHooks::getNumHeapAllocationsOnCurrentThread();
    lastCallImpl->amountHeapAllocatedBeforeFunctionCall = qiti::MallocHooks::getTotalAmountHeapAllocatedOnCurrentThread();
    
    // Grab starting times last without doing additional work after
    lastCallImpl->startTimeWallClock = std::chrono::steady_clock::now();
#ifdef _WIN32
    clock_gettime_windows(lastCallImpl->startTimeCpu);
#else
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &lastCallImpl->startTimeCpu); // last to be most precise
#endif
}

void Profile::updateFunctionDataOnExit(const void* this_fn) noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    auto& functionData = Utils::getFunctionDataFromAddress(this_fn);
    auto* impl = functionData.getImpl();
    auto* callImpl = impl->lastCallData.getImpl();
    timespec cpuEndTime;
    
    // Get end times immediately before doing any other work
#ifdef _WIN32
    clock_gettime_windows(cpuEndTime);
#else
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &cpuEndTime); // first to be most precise
#endif
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
    callImpl->numHeapAllocationsAfterFunctionCall = qiti::MallocHooks::getNumHeapAllocationsOnCurrentThread();
    callImpl->amountHeapAllocatedAfterFunctionCall = qiti::MallocHooks::getTotalAmountHeapAllocatedOnCurrentThread();
    
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
    
    // Update min/max time spent in function (wall clock)
    if(impl->minTimeSpentInFunctionNanosecondsWallClock == 0 || 
       callImpl->timeSpentInFunctionNanosecondsWallClock < impl->minTimeSpentInFunctionNanosecondsWallClock)
    {
        impl->minTimeSpentInFunctionNanosecondsWallClock = callImpl->timeSpentInFunctionNanosecondsWallClock;
    }
    if(callImpl->timeSpentInFunctionNanosecondsWallClock > impl->maxTimeSpentInFunctionNanosecondsWallClock)
    {
        impl->maxTimeSpentInFunctionNanosecondsWallClock = callImpl->timeSpentInFunctionNanosecondsWallClock;
    }
    
    // Update min/max time spent in function (CPU)
    if(impl->minTimeSpentInFunctionNanosecondsCpu == 0 || 
       callImpl->timeSpentInFunctionNanosecondsCpu < impl->minTimeSpentInFunctionNanosecondsCpu)
    {
        impl->minTimeSpentInFunctionNanosecondsCpu = callImpl->timeSpentInFunctionNanosecondsCpu;
    }
    if(callImpl->timeSpentInFunctionNanosecondsCpu > impl->maxTimeSpentInFunctionNanosecondsCpu)
    {
        impl->maxTimeSpentInFunctionNanosecondsCpu = callImpl->timeSpentInFunctionNanosecondsCpu;
    }
    
    // Pop this function from the call stack
    if (! g_callStack.empty())
    {
        g_callStack.pop();
    }
}
} // namespace qiti
