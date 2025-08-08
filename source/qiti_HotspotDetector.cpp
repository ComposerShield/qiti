/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_HotspotDetector.cpp
 *
 * @author   Adam Shield
 * @date     2025-05-16
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "qiti_HotspotDetector.hpp"

#include "qiti_MallocHooks.hpp"
#include "qiti_ScopedNoHeapAllocations.hpp"

#include <algorithm>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

//--------------------------------------------------------------------------

namespace qiti
{

std::vector<HotspotDetector::Hotspot> HotspotDetector::detectHotspots() noexcept
{
    return detectHotspots(0.0); // No threshold - return all functions
}

std::vector<HotspotDetector::Hotspot> HotspotDetector::detectHotspots(double scoreThreshold) noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;
    
    std::vector<Hotspot> hotspots;
    
    // Get all profiled functions
    auto allFunctions = FunctionData::getAllProfiledFunctionData();
    hotspots.reserve(allFunctions.size());
    
    // Analyze each function
    for (const auto* func : allFunctions)
    {
        if (func == nullptr)
            continue;
            
        double score = calculateHotspotScore(func);
        
        // Only include functions above the threshold
        if (score >= scoreThreshold)
        {
            Hotspot hotspot;
            hotspot.function = func;
            hotspot.score = score;
            hotspot.reason = getHotspotReason(func);
            
            hotspots.push_back(std::move(hotspot));
        }
    }
    
    // Sort by score (highest first)
    std::sort(hotspots.begin(), hotspots.end(), 
              [](const Hotspot& a, const Hotspot& b)
              {
                  return a.score > b.score;
              });
    
    return hotspots;
}

double HotspotDetector::calculateHotspotScore(const FunctionData* func) noexcept
{
    if (func == nullptr)
        return 0.0;
    
    // Calculate total time spent = number of calls × average time per call
    uint64_t numCalls = func->getNumTimesCalled();
    uint64_t avgTime = func->getAverageTimeSpentInFunctionCpu_ns();
    
    if (numCalls == 0)
        return 0.0;
    
    // Total time in nanoseconds
    uint64_t totalTime = numCalls * avgTime;
    
    // Convert to double (score represents total nanoseconds)
    return static_cast<double>(totalTime);
}

std::string HotspotDetector::getHotspotReason(const FunctionData* func) noexcept
{
    if (func == nullptr)
        return "Unknown function";
    
    MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;
    
    std::ostringstream reason;
    
    uint64_t numCalls = func->getNumTimesCalled();
    uint64_t avgTimeCpu = func->getAverageTimeSpentInFunctionCpu_ns();
    uint64_t maxTimeCpu = func->getMaxTimeSpentInFunctionCpu_ns();
    uint64_t totalTime = numCalls * avgTimeCpu;
    
    // Primary reason - total time consumption
    reason << "Total time: " << (totalTime / 1000000) << "ms";
    
    // Add details about call frequency
    reason << " (" << numCalls << " calls";
    
    if (numCalls > 0)
    {
        reason << ", avg: " << (avgTimeCpu / 1000) << "μs";
        
        // Highlight if there's high variance (max >> avg)
        if (maxTimeCpu > avgTimeCpu * 3)
        {
            reason << ", max: " << (maxTimeCpu / 1000) << "μs";
        }
    }
    
    reason << ")";
    
    // Add special characteristics
    if (func->getNumExceptionsThrown() > 0)
    {
        reason << " [" << func->getNumExceptionsThrown() << " exceptions]";
    }
    
    if (func->isConstructor())
    {
        reason << " [constructor]";
    }
    else if (func->isDestructor())
    {
        reason << " [destructor]";
    }
    
    return reason.str();
}

} // namespace qiti
