
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
    QITI_API_VAR static thread_local bool bypassMallocHooks;
    QITI_API_VAR static thread_local uint32_t numHeapAllocationsOnCurrentThread;
    QITI_API_VAR static thread_local uint64_t totalAmountHeapAllocatedOnCurrentThread;
    QITI_API_VAR static thread_local uint64_t currentAmountHeapAllocatedOnCurrentThread;
    QITI_API_VAR static thread_local std::function<void()> onNextHeapAllocation;
    
    struct ScopedBypassMallocHooks final
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
     macOS: on every call to malloc via malloc zone hooks
     Linux: on every call to `__sanitizer_malloc_hook` (called from TSan)
     
     Records the allocation size, updates thread-local counters, and executes
     any pending onNextHeapAllocation callback if set.
     
     Custom implementation details ignored if not currently in a Qiti test.
     */
    QITI_API static void mallocHook(std::size_t size) noexcept;
    
    /**
     Hook invoked on each malloc call with pointer tracking for leak detection.
     
     @param ptr Pointer returned by malloc (nullptr if allocation failed)
     @param size Size of allocation
     */
    QITI_API static void mallocHookWithTracking(void* ptr, std::size_t size) noexcept;
    
    /**
     Hook invoked on each free call for leak detection.
     
     @param ptr Pointer to free
     */
    QITI_API static void freeHookWithTracking(void* ptr) noexcept;
    
    /**
     Hook invoked on each realloc call for leak detection.
     
     @param oldPtr Original pointer (may be nullptr)
     @param newPtr New pointer returned by realloc (may be nullptr if failed)
     @param oldSize Size of original allocation (0 if oldPtr was nullptr)
     @param newSize New size requested
     */
    QITI_API static void reallocHookWithTracking(void* oldPtr,
                                                 void* newPtr,
                                                 std::size_t oldSize,
                                                 std::size_t newSize) noexcept;
    

private:
    MallocHooks() = delete;
    ~MallocHooks() = delete;
};
} // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
