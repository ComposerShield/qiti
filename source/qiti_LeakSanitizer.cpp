
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
    auto amountHeapAllocatedBefore = qiti::MallocHooks::totalAmountHeapAllocatedOnCurrentThread;
    
    if (func != nullptr)
        func();
    
    auto amountHeapAllocatedAfter = qiti::MallocHooks::totalAmountHeapAllocatedOnCurrentThread;
    
    if (amountHeapAllocatedAfter != amountHeapAllocatedBefore)
        _passed = false;
}

bool LeakSanitizer::passed() noexcept
{
    return true;  // TODO: implement
}

} // namespace qiti
