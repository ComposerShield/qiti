
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_LockData.hpp
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
        QITI_API_INTERNAL ScopedBypassMallocHooks() noexcept
        : previous(bypassMallocHooks) { bypassMallocHooks = true; }
        
        QITI_API_INTERNAL ~ScopedBypassMallocHooks() noexcept { bypassMallocHooks = previous; }
    private:
        const bool previous;
    };
    
    /** */
    static void QITI_API_INTERNAL mallocHook(std::size_t size) noexcept;
};
} // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
