
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

struct ScopedNoHeapAllocations
{
public:
    ScopedNoHeapAllocations()  noexcept : numHeapAllocationsBefore(Profile::getNumHeapAllocationsOnCurrentThread()) {}
    ~ScopedNoHeapAllocations() noexcept
    {
        auto numHeapAllocationsAfter = Profile::getNumHeapAllocationsOnCurrentThread();
        assert(numHeapAllocationsBefore == numHeapAllocationsAfter);
    }
    
private:
    const uint64_t numHeapAllocationsBefore;
    
    ScopedNoHeapAllocations(const ScopedNoHeapAllocations&) = delete;
    ScopedNoHeapAllocations& operator=(const ScopedNoHeapAllocations&) = delete;
    ScopedNoHeapAllocations(ScopedNoHeapAllocations&&) = delete;
    ScopedNoHeapAllocations& operator=(ScopedNoHeapAllocations&&) = delete;
    
    // Prevent heap allocating this class
    void* operator new(size_t) = delete;
    void* operator new[](size_t) = delete;
};
} // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
