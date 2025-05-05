
#pragma once

#include "qiti_utils.hpp"

//--------------------------------------------------------------------------

#ifndef QITI_DISABLE_INSTRUMENTS
namespace qiti
{
namespace instrument
{
void QITI_API resetInstrumentation() noexcept;

/* Provide a callback on next heap allocation */
void QITI_API onNextHeapAllocation(void(*heapAllocCallback)()) noexcept;
/** shortcut for onNextHeapAllocation([]{assert(false);}); */
void QITI_API assertOnNextHeapAllocation() noexcept;

} // namespace instrument
} // namespace qiti
#endif // ! QITI_DISABLE_INSTRUMENTS
