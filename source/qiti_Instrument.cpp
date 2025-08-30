
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

#include <atomic>
#include <cassert>
#include <functional>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <utility>

//--------------------------------------------------------------------------

extern bool isQitiTestRunning() noexcept;

//--------------------------------------------------------------------------

// Thread-local storage for function call callbacks
inline static thread_local std::unordered_map<const void*, std::function<void()>> g_onNextFunctionCallMap;
// Thread-local storage for thread creation callbacks
inline static std::atomic<std::function<void(std::thread::id)>*> g_onThreadCreationCallback{nullptr};

// Thread creation detection infrastructure
struct ThreadInitializationHook final
{
    QITI_API_INTERNAL ThreadInitializationHook() noexcept
    {
        if (! isQitiTestRunning())
            return;
        
        // Use acquire ordering to ensure we see the complete storedCallback state
        auto threadCreationCallback = g_onThreadCreationCallback.exchange(nullptr, std::memory_order_acquire);
        if (threadCreationCallback != nullptr)
            (*threadCreationCallback)(std::this_thread::get_id());
    }
};

extern "C"
QITI_API_INTERNAL
__attribute__((noinline))
__attribute__((optnone))
void qitiEnsureInstrumentTranslationUnitInitialized() noexcept
{
    static thread_local ThreadInitializationHook qitiThreadInitialized;
}

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
    g_onThreadCreationCallback.store(nullptr, std::memory_order_relaxed);
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

void Instrument::onThreadCreation(std::function<void(std::thread::id threadId)> callback) noexcept
{
    qiti::MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;
    
    // Clear any existing callback first to prevent race condition where a new thread
    // could access a stale pointer between when we update storedCallback and when
    // we update the atomic pointer
    g_onThreadCreationCallback.store(nullptr, std::memory_order_relaxed);
    
    // Store the callback in a static location that the atomic pointer can reference
    static std::function<void(std::thread::id)> storedCallback;
    storedCallback = std::move(callback);
    
    // Set the atomic pointer to point to our stored callback with release ordering
    // to ensure storedCallback update is visible before the pointer
    g_onThreadCreationCallback.store(&storedCallback, std::memory_order_release);
}

void Instrument::checkAndExecuteFunctionCallCallback(const void* functionAddress) noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    auto it = g_onNextFunctionCallMap.find(functionAddress);
    if (it != g_onNextFunctionCallMap.end()) // callback for function found
    {
        it->second(); // Execute callback
        g_onNextFunctionCallMap.erase(it); // Remove after execution
    }
}

} // namespace qiti
