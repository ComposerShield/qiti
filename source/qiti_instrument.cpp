
#include "qiti_instrument.hpp"

#include <cassert>

namespace qiti
{
namespace qiti
{
void onNextHeapAllocation(void (*heapAllocCallback)())
{
    // TODO: implement
}

void assertOnNextHeapAllocation()
{
    onNextHeapAllocation([]{ assert(false); });
}
} // namespace profile
} // namespace qiti
