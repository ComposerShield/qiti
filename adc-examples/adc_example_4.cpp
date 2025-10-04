
/******************************************************************************
 * Tracking total bytes allocated with size-per-pointer tracking
 ******************************************************************************/

#include <new>
#include <cstdlib>
#include <unordered_map>

static std::size_t bytesCurrentlyAllocated = 0;
static std::size_t numHeapAllocations = 0;
static std::unordered_map<void*, std::size_t> allocationSizes;
static bool shouldTrackAllocation = true;

void* operator new(std::size_t size)
{
    void* ptr = std::malloc(size);
    
    if (! ptr)
        throw std::bad_alloc();
    
    if (shouldTrackAllocation)
    {
        shouldTrackAllocation = false;
        bytesCurrentlyAllocated += size;
        numHeapAllocations++;
        allocationSizes[ptr] = size;
        shouldTrackAllocation = true;
    }
    
    return ptr;
}

void operator delete(void* ptr) noexcept
{
    if (shouldTrackAllocation)
    {
        shouldTrackAllocation = false;
        auto it = allocationSizes.find(ptr);
        if (it != allocationSizes.end())
        {
            bytesCurrentlyAllocated -= it->second;
            allocationSizes.erase(it);
        }
        shouldTrackAllocation = true;
    }
    
    std::free(ptr);
}
