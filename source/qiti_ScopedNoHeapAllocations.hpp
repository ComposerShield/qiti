
// Copyright (c) 2025 Adam Shield
// SPDX-License-Identifier: MIT

#include <cassert>
#include <cstdint>

namespace qiti
{
namespace profile
{
unsigned long long getNumHeapAllocationsOnCurrentThread() noexcept;
} // namespace profile

struct ScopedNoHeapAllocations
{
public:
    ScopedNoHeapAllocations()  noexcept : numHeapAllocationsBefore(profile::getNumHeapAllocationsOnCurrentThread()) {}
    ~ScopedNoHeapAllocations() noexcept
    {
        auto numHeapAllocationsAfter = profile::getNumHeapAllocationsOnCurrentThread();
        assert(numHeapAllocationsBefore == numHeapAllocationsAfter);
    }
    
private:
    const unsigned long long numHeapAllocationsBefore;
    
    ScopedNoHeapAllocations(const ScopedNoHeapAllocations&) = delete;
    ScopedNoHeapAllocations& operator=(const ScopedNoHeapAllocations&) = delete;
    ScopedNoHeapAllocations(ScopedNoHeapAllocations&&) = delete;
    ScopedNoHeapAllocations& operator=(ScopedNoHeapAllocations&&) = delete;
    
    // Prevent heap allocating this class
    void* operator new(size_t) = delete;
    void* operator new[](size_t) = delete;
};
} // namespace qiti
