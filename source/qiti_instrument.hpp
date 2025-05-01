
#pragma once

#include "qiti_utils.hpp"

namespace qiti
{
namespace qiti
{
/* Provide a callback on next heap allocation */
void QITI_API onNextHeapAllocation(void(*heapAllocCallback)());
/** shortcut for onNextHeapAllocation([]{assert(false);}); */
void QITI_API assertOnNextHeapAllocation();

} // namespace profile
} // namespace qiti
