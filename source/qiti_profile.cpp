
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

std::unordered_set<void*> g_functionsToProfile;
bool g_profileAllFunctions = false;

struct Init_g_functionsToProfile
{
    Init_g_functionsToProfile()
    {
        g_functionsToProfile.reserve(256); // automatically reserve on startup
    }
};
static const Init_g_functionsToProfile init_g_functionsToProfile;

inline thread_local uint64_t g_numHeapAllocationsOnCurrentThread = 0;

extern thread_local std::function<void()> g_onNextHeapAllocation;
extern std::recursive_mutex qiti_global_lock;

//--------------------------------------------------------------------------

void* QITI_API malloc(size_t __size);

void* QITI_API operator new(size_t size)
{
    ++g_numHeapAllocationsOnCurrentThread;
    if (g_onNextHeapAllocation != nullptr)
    {
        g_onNextHeapAllocation();
        g_onNextHeapAllocation = nullptr;
    }
    
    // Original implementation
    void* p = malloc(size);
    return p;
}

//--------------------------------------------------------------------------

/** */
static void QITI_API_INTERNAL updateFunctionType(qiti::FunctionData& functionData) noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;

    auto* impl = functionData.getImpl();
    const char* name = impl->functionNameReal;
    std::string_view sv{name};

    // find the opening '(' of the parameter list
    auto paren = sv.find('(');
    if (paren != std::string_view::npos)
    {
        // strip off the "(" and everything after:
        sv.remove_suffix(sv.size() - paren);

        // now find the last "::" in the prefix:
        auto colcol = sv.rfind("::");
        if (colcol != std::string_view::npos) {
            // [qualifier]::[last]
            std::string_view qualifier = sv.substr(0, colcol);
            std::string_view last_part = sv.substr(colcol + 2);

            // constructor: qualifier == last_part
            if (qualifier == last_part)
            {
                impl->functionType = qiti::FunctionType::constructor;
            }
            // destructor: last_part begins with '~' and qualifier == last_part.substr(1)
            else if (!last_part.empty() && last_part[0] == '~'
                     && qualifier == last_part.substr(1))
            {
                impl->functionType = qiti::FunctionType::destructor;
            }
        }
    }
}

//--------------------------------------------------------------------------

namespace qiti
{
namespace profile
{
void resetProfiling() noexcept
{
    // Prevent any qiti work while we disable profiling
    std::scoped_lock<std::recursive_mutex> lock(qiti_global_lock);
        
    g_functionsToProfile.clear();
    g_profileAllFunctions = false;
    g_numHeapAllocationsOnCurrentThread = 0;
}

void beginProfilingFunction(void* functionAddress) noexcept
{
    g_functionsToProfile.insert(functionAddress);
    
    // This adds the function to our function map
    (void)qiti::getFunctionDataFromAddress(functionAddress);
}

void endProfilingFunction(void* functionAddress) noexcept
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

bool isProfilingFunction(void* funcAddress) noexcept
{
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
    return g_numHeapAllocationsOnCurrentThread;
}

void updateFunctionDataOnEnter(void* this_fn) noexcept
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
#ifndef QITI_DISABLE_HEAP_ALLOCATION_TRACKER
    impl->lastCallData.getImpl()->numHeapAllocationsBeforeFunctionCall = g_numHeapAllocationsOnCurrentThread;
#endif
    updateFunctionType(functionData);
}

void updateFunctionDataOnExit(void* this_fn) noexcept
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
#ifndef QITI_DISABLE_HEAP_ALLOCATION_TRACKER
    callImpl->numHeapAllocationsAfterFunctionCall = g_numHeapAllocationsOnCurrentThread;
#endif
}
} // namespace profile
} // namespace qiti
