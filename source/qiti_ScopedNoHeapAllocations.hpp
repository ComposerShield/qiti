
#include <cassert>

extern thread_local unsigned long long g_numHeapAllocationsOnCurrentThread;

namespace qiti
{
struct ScopedNoHeapAllocations
{
public:
    ScopedNoHeapAllocations()  noexcept : numHeapAllocationsBefore(g_numHeapAllocationsOnCurrentThread) {}
    ~ScopedNoHeapAllocations() noexcept
    {
        auto numHeapAllocationsAfter = g_numHeapAllocationsOnCurrentThread;
        assert(numHeapAllocationsBefore == numHeapAllocationsAfter);
    }
    
private:
    const unsigned long long numHeapAllocationsBefore;
    
    ScopedNoHeapAllocations(const ScopedNoHeapAllocations&) = delete;
    ScopedNoHeapAllocations& operator=(const ScopedNoHeapAllocations&) = delete;
    ScopedNoHeapAllocations(ScopedNoHeapAllocations&&) = delete;
    ScopedNoHeapAllocations& operator=(ScopedNoHeapAllocations&&) = delete;
    
    // Prevent heap allocating this class
    void* operator new(std::size_t) = delete;
    void* operator new[](std::size_t) = delete;
};
} // namespace qiti
