
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
thread_local uint32_t qiti::MallocHooks::numHeapAllocationsOnCurrentThread = 0;
thread_local uint64_t qiti::MallocHooks::amountHeapAllocatedOnCurrentThread;
thread_local std::function<void()> qiti::MallocHooks::onNextHeapAllocation = nullptr;

void qiti::MallocHooks::mallocHook(std::size_t size) noexcept
{
    if (! isQitiTestRunning())
        return;
    
    if (qiti::MallocHooks::bypassMallocHooks)
        return;
    
    ++numHeapAllocationsOnCurrentThread;
    amountHeapAllocatedOnCurrentThread += size;

    if (onNextHeapAllocation != nullptr)
    {
        onNextHeapAllocation();
        onNextHeapAllocation = nullptr;
    }
}

//--------------------------------------------------------------------------

#if defined(__APPLE__)
/**
 Due to differences in the Thread Sanitizer runtime on Apple vs. Linux,
 we need to insert our logic into "operator new" in macOS from the Qiti
 dylib but on Linux, we must insert it into the malloc hook provided by
 Thread Sanitizer directly in the final executable (qiti_tests_client.cpp).
 */
void* operator new(std::size_t size)
{
    qiti::MallocHooks::mallocHook(size);
    
    // Original implementation
    if (void* ptr = std::malloc(size))
        return ptr;
    
    throw std::bad_alloc{};
}

void* operator new[](std::size_t size)
{
    qiti::MallocHooks::mallocHook(size);
    
    if (size == 0)
        ++size; // avoid std::malloc(0) which may return nullptr on success
 
    if (void* ptr = std::malloc(size))
        return ptr;
 
    throw std::bad_alloc{};
}
#endif // defined(__APPLE__)

//--------------------------------------------------------------------------
