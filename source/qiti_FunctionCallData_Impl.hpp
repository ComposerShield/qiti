/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_FunctionCallData_Impl.hpp
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

#include "qiti_FunctionCallData.hpp"

#include <time.h>
#include <stdint.h>

#include <chrono>
#include <cstdint>
#include <thread>

//--------------------------------------------------------------------------
// Doxygen - Begin Internal Documentation
/** \cond INTERNAL */
//--------------------------------------------------------------------------

namespace qiti
{
struct FunctionCallData::Impl
{
    std::chrono::steady_clock::time_point startTimeWallClock;
    std::chrono::steady_clock::time_point endTimeWallClock;
    timespec                              startTimeCpu;
    timespec                              endTimeCpu;
    
    std::thread::id callingThread;
    const FunctionData* caller = nullptr;
    
    uint64_t timeSpentInFunctionNanosecondsWallClock = 0;
    uint64_t timeSpentInFunctionNanosecondsCpu = 0;
    
    uint32_t numHeapAllocationsBeforeFunctionCall = 0;
    uint32_t numHeapAllocationsAfterFunctionCall  = 0;
    
    uint64_t amountHeapAllocatedBeforeFunctionCall = 0;
    uint64_t amountHeapAllocatedAfterFunctionCall  = 0;
    
    uint64_t numExceptionsThrown = 0;
};
} // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
