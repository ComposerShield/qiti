
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_instrument.cpp
 *
 * @author   Adam Shield
 * @date     2025-05-16
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "qiti_instrument.hpp"

#include "qiti_MallocHooks.hpp"
#include "qiti_ScopedNoHeapAllocations.hpp"

#include <cassert>
#include <functional>
#include <mutex>

//--------------------------------------------------------------------------

#ifndef QITI_DISABLE_INSTRUMENTS
extern std::recursive_mutex qiti_global_lock;

namespace qiti
{
namespace instrument
{
void resetInstrumentation() noexcept
{
    // Prevent any qiti work while we clear out instrumenting of functions
    std::scoped_lock<std::recursive_mutex> lock(qiti_global_lock);
    
    qiti::ScopedNoHeapAllocations noAlloc;
    
    MallocHooks::onNextHeapAllocation = nullptr;
}

void onNextHeapAllocation(void (*heapAllocCallback)()) noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    MallocHooks::onNextHeapAllocation = heapAllocCallback;
}

void assertOnNextHeapAllocation() noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    onNextHeapAllocation([]{ assert(false); });
}
} // namespace instrument
} // namespace qiti
#endif // ! QITI_DISABLE_INSTRUMENTS
