
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_MallocHooks.hpp
 *
 * @author   Adam Shield
 * @date     2025-05-25
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#pragma once

#include "qiti_API.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>

//--------------------------------------------------------------------------
// Doxygen - Begin Internal Documentation
/** \cond INTERNAL */
//--------------------------------------------------------------------------

namespace qiti
{
//--------------------------------------------------------------------------
/**
 */
class MallocHooks
{
public:
    static thread_local bool bypassMallocHooks;
    static thread_local uint32_t numHeapAllocationsOnCurrentThread;
    static thread_local uint64_t amountHeapAllocatedOnCurrentThread;
    static thread_local std::function<void()> onNextHeapAllocation;
    
    struct ScopedBypassMallocHooks
    {
        /** Temporarily disable malloc hooks for the current thread for however long this object is in scope. */
        inline QITI_API_INTERNAL ScopedBypassMallocHooks() noexcept
        : previous(bypassMallocHooks) { bypassMallocHooks = true; }
        
        /** On destruction, resets bypassMallocHooks to the value saved at construction. */
        inline QITI_API_INTERNAL ~ScopedBypassMallocHooks() noexcept { bypassMallocHooks = previous; }
    private:
        const bool previous;
    };
    
    /**
     Hook invoked on each malloc call.
     
     More specifically,
     macOs: on every call to `operator new` or `operator new[]`
     Linux: on every call to `__sanitizer_malloc_hook` (called from TSan)
     
     Records the allocation size, updates thread-local counters, and executes
     any pending onNextHeapAllocation callback if set.
     
     Custom implementation details ignored if not currently in a Qiti test.
     */
    static void QITI_API mallocHook(std::size_t size) noexcept;

private:
    MallocHooks() = delete;
    ~MallocHooks() = delete;
};
} // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
