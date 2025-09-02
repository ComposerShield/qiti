
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
 Memory allocation hook management and instrumentation utilities.

 The MallocHooks class provides a centralized interface for intercepting and
 tracking memory allocations during test execution. It maintains thread-local
 state for allocation counting and provides mechanisms to temporarily bypass
 hooks when needed (e.g., during internal Qiti operations).

 @note This class is designed for internal use by the Qiti profiling system.
 */
class MallocHooks
{
public:
    // Accessor functions for global thread_local variables
    [[nodiscard]] QITI_API static bool& getBypassMallocHooks() noexcept;
    [[nodiscard]] QITI_API static uint32_t& getNumHeapAllocationsOnCurrentThread() noexcept;
    [[nodiscard]] QITI_API static uint64_t& getTotalAmountHeapAllocatedOnCurrentThread() noexcept;
    [[nodiscard]] QITI_API static uint64_t& getCurrentAmountHeapAllocatedOnCurrentThread() noexcept;
    [[nodiscard]] QITI_API static std::function<void()>& getOnNextHeapAllocation() noexcept;
    
    /**
     RAII guard for temporarily disabling malloc hooks on the current thread.

     This class provides a safe mechanism to bypass malloc tracking during
     critical sections where allocation hooks might interfere with internal
     operations (e.g., when the hook implementation itself needs to allocate
     memory for logging or data structures).

     The bypass state is automatically restored when the object goes out of
     scope, ensuring thread-local state consistency even in the presence of
     exceptions.

     @note This class is designed for internal use by the Qiti profiling system.
     */
    struct ScopedBypassMallocHooks final
    {
        /** Temporarily disable malloc hooks for the current thread for however long this object is in scope. */
        QITI_API_INTERNAL ScopedBypassMallocHooks() noexcept
        : previous(getBypassMallocHooks()) { getBypassMallocHooks() = true; }
        
        /** On destruction, resets bypassMallocHooks to the value saved at construction. */
        QITI_API_INTERNAL ~ScopedBypassMallocHooks() noexcept { getBypassMallocHooks() = previous; }
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
    
    // Deleted constructors/destructors
    MallocHooks() = delete;
    ~MallocHooks() = delete;
};
} // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
