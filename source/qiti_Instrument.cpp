
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
#include "qiti_ScopedNoHeapAllocations.hpp"

#include <cassert>
#include <functional>
#include <mutex>

//--------------------------------------------------------------------------

#ifndef QITI_DISABLE_INSTRUMENTS
namespace qiti
{
void Instrument::resetInstrumentation() noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    MallocHooks::getOnNextHeapAllocation() = nullptr;
}

void Instrument::onNextHeapAllocation(void (*heapAllocCallback)()) noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    MallocHooks::getOnNextHeapAllocation() = heapAllocCallback;
}

void Instrument::assertOnNextHeapAllocation() noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    onNextHeapAllocation([]{ assert(false); });
}
} // namespace qiti
#endif // ! QITI_DISABLE_INSTRUMENTS
