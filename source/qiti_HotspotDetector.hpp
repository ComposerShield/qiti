/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_HotspotDetector.hpp
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

#include "qiti_API.hpp"
#include "qiti_FunctionData.hpp"

#include <string>
#include <vector>

//--------------------------------------------------------------------------

namespace qiti
{
//--------------------------------------------------------------------------

/**
 Analyzes profiling data to identify performance hotspots.
 
 Uses call frequency and timing data to automatically detect functions that
 contribute most significantly to overall execution time.
 */
class HotspotDetector
{
public:
    /**
     Represents a detected performance hotspot.
     */
    struct Hotspot
    {
        const FunctionData* function = nullptr;  ///< The function identified as a hotspot
        double score = 0.0;                      ///< Hotspot score (higher = more significant)
        std::string reason;                      ///< Human-readable explanation of why this is a hotspot
    };
    
    /**
     @returns A vector of detected hotspots, sorted by score (highest first).
     
     Analyzes all currently profiled functions and identifies those that consume
     the most execution time based on call frequency and average execution time.
     Functions with higher total time consumption receive higher scores.
     */
    [[nodiscard]] static std::vector<Hotspot> QITI_API detectHotspots() noexcept;
    
    /**
     @returns A vector of detected hotspots above the specified threshold.
     
     @param scoreThreshold Minimum score required for a function to be considered a hotspot.
     Functions with scores below this threshold will be filtered out.
     */
    [[nodiscard]] static std::vector<Hotspot> QITI_API detectHotspots(double scoreThreshold) noexcept;
    
private:
    /**
     Calculate the hotspot score for a given function.
     
     @param func The function to analyze
     @returns A hotspot score based on total time consumption (calls × average time)
     */
    [[nodiscard]] static double QITI_API_INTERNAL calculateHotspotScore(const FunctionData* func) noexcept;
    
    /**
     Generate a human-readable explanation for why a function is considered a hotspot.
     
     @param func The function to analyze
     @returns A descriptive string explaining the hotspot characteristics
     */
    [[nodiscard]] static std::string QITI_API_INTERNAL getHotspotReason(const FunctionData* func) noexcept;
};

//--------------------------------------------------------------------------
} // namespace qiti
//--------------------------------------------------------------------------
