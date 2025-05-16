
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

//--------------------------------------------------------------------------

#ifndef QITI_DISABLE_INSTRUMENTS
namespace qiti
{
namespace instrument
{
void QITI_API resetInstrumentation() noexcept;

/** Provide a callback on next heap allocation */
void QITI_API onNextHeapAllocation(void(*heapAllocCallback)()) noexcept;
/** Dhortcut for onNextHeapAllocation([]{assert(false);}); */
void QITI_API assertOnNextHeapAllocation() noexcept;

} // namespace instrument
} // namespace qiti
#endif // ! QITI_DISABLE_INSTRUMENTS
