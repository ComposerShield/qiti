
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_instrument.hpp
 *
 * @author   Adam Shield
 * @date     2025-05-16
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#pragma once

#include "qiti_API.hpp"

#ifndef QITI_DISABLE_INSTRUMENTS
namespace qiti
{
/**
 Tools for injecting custom logic into the application at runtime.

 The instrument namespace provides functions to hook into the runtime flow,
 allowing you to insert callbacks and additional logic as needed.
 */
namespace instrument
{
/**
 Reset all instrumentation counters and state.

 Clears any recorded data from previous profiling, returning the
 instrumentation subsystem to its initial state.
 */
void QITI_API resetInstrumentation() noexcept;

/**
 Register a callback to be invoked on the next heap allocation.

 Schedules the provided function pointer 'heapAllocCallback' to be
 executed immediately after the next heap allocation occurs. This
 facilitates custom handling or assertions around allocation events.
 */
void QITI_API onNextHeapAllocation(void(*heapAllocCallback)()) noexcept;

/**
 Triggers an assertion failure when the next heap allocation happens.

 Shortcut equivalent to onNextHeapAllocation([]{ assert(false); });
 */
void QITI_API assertOnNextHeapAllocation() noexcept;

} // namespace instrument
} // namespace qiti
#endif // ! QITI_DISABLE_INSTRUMENTS
