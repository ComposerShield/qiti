
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_LeakSanitizer.cpp
 *
 * @author   Adam Shield
 * @date     2025-05-16
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "qiti_LeakSanitizer.hpp"

#include "qiti_MallocHooks.hpp"
#include "qiti_Profile.hpp"

namespace qiti
{
LeakSanitizer::LeakSanitizer() noexcept
{
    
}

LeakSanitizer::~LeakSanitizer() noexcept
{
    
}

void LeakSanitizer::run(std::function<void()> func) noexcept
{
    uint64_t amountHeapAllocatedBefore;
    uint64_t amountHeapAllocatedAfter;
    
    {
        qiti::Profile::ScopedDisableProfiling disableProfiling;
        
        amountHeapAllocatedBefore = qiti::MallocHooks::currentAmountHeapAllocatedOnCurrentThread;
    } // ScopedDisableProfiling goes out of scope, re-enable profiling during user function execution
    
    if (func != nullptr)
    {
        func();
    }
    
    {
        qiti::Profile::ScopedDisableProfiling disableProfiling;
        
        // Any new heap allocations should be freed by the end of the function so this value should match.
        amountHeapAllocatedAfter = qiti::MallocHooks::currentAmountHeapAllocatedOnCurrentThread;
        
        if (amountHeapAllocatedAfter != amountHeapAllocatedBefore)
            _passed = false;
    }
}

bool LeakSanitizer::passed() noexcept
{
    return _passed;
}

LeakSanitizer::LeakSanitizer(LeakSanitizer&& other) noexcept
    : _passed(other._passed.load())
{
}

LeakSanitizer& LeakSanitizer::operator=(LeakSanitizer&& other) noexcept
{
    if (this != &other)
    {
        _passed = other._passed.load();
    }
    return *this;
}

} // namespace qiti
