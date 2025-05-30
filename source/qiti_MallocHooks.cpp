
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

#include "qiti_MallocHooks.hpp"

//--------------------------------------------------------------------------

extern bool isQitiTestRunning() noexcept;

//--------------------------------------------------------------------------

thread_local bool qiti::MallocHooks::bypassMallocHooks = false;

thread_local uint64_t qiti::MallocHooks::g_numHeapAllocationsOnCurrentThread = 0;

thread_local std::function<void()> qiti::MallocHooks::g_onNextHeapAllocation = nullptr;

bool qiti::MallocHooks::getBypassMallocHooks()
{
    return bypassMallocHooks;
}

void qiti::MallocHooks::mallocHook() noexcept
{
    if (! isQitiTestRunning())
        return;
    
    if (qiti::MallocHooks::bypassMallocHooks)
        return;
    
    ++qiti::MallocHooks::g_numHeapAllocationsOnCurrentThread;
    if (qiti::MallocHooks::g_onNextHeapAllocation != nullptr)
    {
        qiti::MallocHooks::g_onNextHeapAllocation();
        qiti::MallocHooks::g_onNextHeapAllocation = nullptr;
    }
}

#if defined(__APPLE__)
void* operator new([[maybe_unused]] std::size_t size)
{
    qiti::MallocHooks::mallocHook();
    
    if (void* ptr = std::malloc(size))
        return ptr;
    
    throw std::bad_alloc{};
}
#endif // defined(__APPLE__)

//--------------------------------------------------------------------------
