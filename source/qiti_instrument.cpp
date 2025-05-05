
#include "qiti_instrument.hpp"

#include <cassert>
#include <functional>

//--------------------------------------------------------------------------

#ifndef QITI_DISABLE_INSTRUMENTS
/** Note: not static, external linkage so it can be used in other translation units. */
thread_local std::function<void()> g_onNextHeapAllocation = nullptr;

extern std::recursive_mutex qiti_global_lock;

namespace qiti
{
namespace instrument
{
void resetInstrumentation() noexcept
{
    // Prevent any qiti work while we clear out instrumenting of functions
    std::scoped_lock<std::recursive_mutex> lock(qiti_global_lock);
    
    g_onNextHeapAllocation = nullptr;
}

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
