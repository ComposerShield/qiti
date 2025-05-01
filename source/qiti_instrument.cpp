
#include "qiti_instrument.hpp"

#include <cassert>
#include <functional>

//--------------------------------------------------------------------------

#ifndef QITI_DISABLE_INSTRUMENTS
/** Note: not static, external linkage so it can be used in other translation units. */
thread_local std::function<void()> g_onNextHeapAllocation = nullptr;

namespace qiti
{
namespace instrument
{
void onNextHeapAllocation(void (*heapAllocCallback)()) noexcept
{
    g_onNextHeapAllocation = heapAllocCallback;
}

void assertOnNextHeapAllocation() noexcept
{
    onNextHeapAllocation([]{ assert(false); });
}
} // namespace instrument
} // namespace qiti
#endif // ! QITI_DISABLE_INSTRUMENTS
