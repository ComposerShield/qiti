
/******************************************************************************
 * Tracking total bytes allocated
 ******************************************************************************/

#include <new>
#include <cstdlib>

static std::size_t totalBytesAllocated = 0;

void* operator new(std::size_t size)
{
    void* ptr = std::malloc(size);
    
    if (! ptr)
        throw std::bad_alloc();
    
    totalBytesAllocated += size;
    
    return ptr;
}

void operator delete(void* ptr) noexcept
{
    std::free(ptr);
}
