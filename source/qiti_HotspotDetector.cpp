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
#include "qiti_Profile.hpp"
#include "qiti_ScopedNoHeapAllocations.hpp"

#include <algorithm>
#include <ranges>
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

std::vector<HotspotDetector::Hotspot> HotspotDetector::detectHotspots(Sensitivity sensitivity) noexcept
{
    // First get all hotspots sorted by score
    auto allHotspots = detectHotspots(0.0);
    
    if (allHotspots.empty() || sensitivity == Sensitivity::ALL)
        return allHotspots;
    
    // Calculate how many functions to return based on sensitivity level
    size_t numToReturn = 0;
    
    switch (sensitivity)
    {
        case Sensitivity::LOW:
            numToReturn = std::max(size_t{1}, allHotspots.size() / 10);  // Top 10% (at least 1)
            break;
        case Sensitivity::MEDIUM:
            numToReturn = std::max(size_t{1}, allHotspots.size() / 4);   // Top 25% (at least 1)
            break;
        case Sensitivity::HIGH:
            numToReturn = std::max(size_t{1}, allHotspots.size() / 2);   // Top 50% (at least 1)
            break;
        case Sensitivity::ALL:
            numToReturn = allHotspots.size();
            break;
    }
    
    // Return the top N functions
    if (numToReturn >= allHotspots.size())
        return allHotspots;
    
    std::vector<Hotspot> result;
    result.reserve(numToReturn);
    for (size_t i = 0; i < numToReturn; ++i)
    {
        result.push_back(allHotspots[i]);
    }
    
    return result;
}

std::vector<HotspotDetector::Hotspot> HotspotDetector::detectHotspots(double scoreThreshold) noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
    qiti::MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;
    
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
    std::ranges::sort(hotspots, 
                      [](const Hotspot& a, const Hotspot& b)
                      {
                          return a.score > b.score;
                      });
    
    return hotspots;
}

double HotspotDetector::calculateHotspotScore(const FunctionData* func) noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
    qiti::ScopedNoHeapAllocations noAllocs;
    
    if (func == nullptr)
        return 0.0;
    
    // Calculate total time spent = number of calls × average time per call
    uint64_t numCalls = func->getNumTimesCalled();
#ifdef _WIN32 // CPU Time feature not supported on Windows
    uint64_t avgTime = func->getAverageTimeSpentInFunctionWallClock_ns();
#else
    uint64_t avgTime = func->getAverageTimeSpentInFunctionCpu_ns();
#endif
    
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
    
    qiti::MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;
    
    std::ostringstream reason;
    
    uint64_t numCalls = func->getNumTimesCalled();
#ifdef _WIN32 // CPU Time feature not supported on Windows
    uint64_t avgTime = func->getAverageTimeSpentInFunctionWallClock_ns();
    uint64_t maxTime = func->getMaxTimeSpentInFunctionWallClock_ns();
#else
    uint64_t avgTime = func->getAverageTimeSpentInFunctionCpu_ns();
    uint64_t maxTime = func->getMaxTimeSpentInFunctionCpu_ns();
#endif
    uint64_t totalTime = numCalls * avgTime;
    
    // Primary reason - total time consumption
    reason << "Total time: " << (totalTime / 1000000) << "ms";
    
    // Add details about call frequency
    reason << " (" << numCalls << " calls";
    
    if (numCalls > 0)
    {
        reason << ", avg: " << (avgTime / 1000) << "μs";
        
        // Highlight if there's high variance (max >> avg)
        if (maxTime > avgTime * 3)
            reason << ", max: " << (maxTime / 1000) << "μs";
    }
    
    reason << ")";
    
    // Add special characteristics
    if (func->getNumExceptionsThrown() > 0)
        reason << " [" << func->getNumExceptionsThrown() << " exceptions]";
    
    if (func->isConstructor())
        reason << " [constructor]";
    else if (func->isDestructor())
        reason << " [destructor]";
    
    return reason.str();
}

} // namespace qiti
