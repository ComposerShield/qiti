
/******************************************************************************
 * Operator new override that maintains original behavior
 ******************************************************************************/

#include <new>
#include <cstdlib>

void* operator new(std::size_t size)
{
    void* ptr = std::malloc(size);
    
    if (! ptr)
        throw std::bad_alloc();
    
    return ptr;
}

void operator delete(void* ptr) noexcept
{
    std::free(ptr);
}
