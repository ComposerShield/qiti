
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

#include <sstream>

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
    // Reset stats for this run
    _totalAllocated = 0;
    _totalDeallocated = 0;
    _netLeak = 0;
    
    uint64_t amountHeapAllocatedBefore;
    uint64_t amountHeapAllocatedAfter;
    uint64_t totalAllocatedBefore;
    uint64_t totalAllocatedAfter;
    
    {
        qiti::Profile::ScopedDisableProfiling disableProfiling;
        
        amountHeapAllocatedBefore = qiti::MallocHooks::currentAmountHeapAllocatedOnCurrentThread;
        totalAllocatedBefore = qiti::MallocHooks::totalAmountHeapAllocatedOnCurrentThread;
    } // ScopedDisableProfiling goes out of scope, re-enable profiling during user function execution
    
    if (func != nullptr)
    {
        func();
    }
    
    {
        qiti::Profile::ScopedDisableProfiling disableProfiling;
        
        // Any new heap allocations should be freed by the end of the function so this value should match.
        amountHeapAllocatedAfter = qiti::MallocHooks::currentAmountHeapAllocatedOnCurrentThread;
        totalAllocatedAfter = qiti::MallocHooks::totalAmountHeapAllocatedOnCurrentThread;
        
        // Calculate allocations that happened during this run
        _totalAllocated = (totalAllocatedAfter - totalAllocatedBefore);
        _netLeak = (amountHeapAllocatedAfter - amountHeapAllocatedBefore);
        _totalDeallocated = _totalAllocated - _netLeak;
        
        if (_netLeak != 0)
            _passed = false;
    }
}

bool LeakSanitizer::passed() const noexcept
{
    return _passed;
}

std::string LeakSanitizer::getReport() const noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
    
    std::ostringstream report;
    report << "LeakSanitizer Report:\n";
    report << "  Total allocated: " << _totalAllocated << " bytes\n";
    report << "  Total deallocated: " << _totalDeallocated << " bytes\n";
    report << "  Net leak: " << _netLeak << " bytes\n";
    report << "  Status: " << (passed() ? "PASSED" : "FAILED");
    
    if (_netLeak > 0)
        report << " (Memory leak detected)";
    else if (_netLeak < 0)
        report << " (More memory freed than allocated - possible double free)";
    
    return report.str();
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
