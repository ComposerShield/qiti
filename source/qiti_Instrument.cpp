
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_Instrument.cpp
 *
 * @author   Adam Shield
 * @date     2025-05-16
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "qiti_Instrument.hpp"

#include "qiti_MallocHooks.hpp"
#include "qiti_Profile.hpp"
#include "qiti_ScopedNoHeapAllocations.hpp"

#include <cassert>
#include <functional>
#include <mutex>
#include <unordered_map>

//--------------------------------------------------------------------------

// Thread-local storage for function call callbacks
static thread_local std::unordered_map<const void*, std::function<void()>> g_onNextFunctionCallMap;

namespace qiti
{
void Instrument::onNextFunctionCallInternal(std::function<void()> callback, const void* functionAddress) noexcept
{
    g_onNextFunctionCallMap[functionAddress] = std::move(callback);
}

void Instrument::resetInstrumentation() noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    MallocHooks::getOnNextHeapAllocation() = nullptr;
    g_onNextFunctionCallMap.clear();
}

void Instrument::onNextHeapAllocation(std::function<void()> heapAllocCallback) noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    MallocHooks::getOnNextHeapAllocation() = std::move(heapAllocCallback);
}

void Instrument::assertOnNextHeapAllocation() noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    onNextHeapAllocation([]{ assert(false); });
}

void Instrument::checkAndExecuteFunctionCallCallback(const void* functionAddress) noexcept
{
    auto it = g_onNextFunctionCallMap.find(functionAddress);
    if (it != g_onNextFunctionCallMap.end()) // callback for function found
    {
        it->second(); // Execute callback
        g_onNextFunctionCallMap.erase(it); // Remove after execution
    }
}

} // namespace qiti
