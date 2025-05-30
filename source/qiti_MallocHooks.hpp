
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

namespace qiti
{
/**
 */
class MallocHooks
{
public:
    QITI_API_VAR static thread_local bool bypassMallocHooks;
    QITI_API_VAR static thread_local uint64_t g_numHeapAllocationsOnCurrentThread;
    QITI_API_VAR static thread_local std::function<void()> g_onNextHeapAllocation;
    
    struct ScopedBypassMallocHooks
    {
        QITI_API ScopedBypassMallocHooks() noexcept
        : previous(bypassMallocHooks) { bypassMallocHooks = true; }
        
        QITI_API ~ScopedBypassMallocHooks() noexcept { bypassMallocHooks = previous; }
    private:
        const bool previous;
    };
    
    __attribute__((no_sanitize_thread))
    static bool QITI_API getBypassMallocHooks();
    
    static void QITI_API mallocHook() noexcept;
};
} // namespace qiti
