
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_ScopedNoHeapAllocations.hpp
 *
 * @author   Adam Shield
 * @date     2025-05-16
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#pragma once

#include "qiti_Profile.hpp"

#include <stddef.h>  // defines size_t in the global namespace

#include <cassert>
#include <cstdint>

//--------------------------------------------------------------------------
// Doxygen - Begin Internal Documentation
/** \cond INTERNAL */
//--------------------------------------------------------------------------

namespace qiti
{
/**
 RAII guard that asserts no heap allocations occur within its scope.
 
 When a ScopedNoHeapAllocations object is constructed, it snapshots the
 current number of heap allocations on the current thread. Upon destruction,
 it queries the number again and asserts that the two values are equal.
 
 This is useful for profiling and for ensuring that critical code paths
 remain allocation-free.
 */
struct ScopedNoHeapAllocations
{
public:
    /** */
    ScopedNoHeapAllocations()  noexcept : numHeapAllocationsBefore(Profile::getNumHeapAllocationsOnCurrentThread()) {}
    /** */
    ~ScopedNoHeapAllocations() noexcept
    {
        auto numHeapAllocationsAfter = Profile::getNumHeapAllocationsOnCurrentThread();
        assert(numHeapAllocationsBefore == numHeapAllocationsAfter);
    }
    
private:
    /// Heap allocation count at construction time.
    const uint64_t numHeapAllocationsBefore;
    
    // Disabled copy/move constructors/assignment operators
    ScopedNoHeapAllocations(const ScopedNoHeapAllocations&) = delete;
    ScopedNoHeapAllocations& operator=(const ScopedNoHeapAllocations&) = delete;
    ScopedNoHeapAllocations(ScopedNoHeapAllocations&&) = delete;
    ScopedNoHeapAllocations& operator=(ScopedNoHeapAllocations&&) = delete;
    
    // Prevent this guard itself from being heap‐allocated
    void* operator new(size_t) = delete;
    void* operator new[](size_t) = delete;
};
} // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
