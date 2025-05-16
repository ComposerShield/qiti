
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

#include <chrono>
#include <cstdint>
#include <thread>

namespace qiti
{
struct FunctionCallData::Impl
{
    std::chrono::steady_clock::time_point begin_time;
    std::chrono::steady_clock::time_point end_time;
    std::thread::id callingThread;
    
    uint timeSpentInFunctionNanoseconds = 0;
    
    uint numHeapAllocationsBeforeFunctionCall = 0;
    uint numHeapAllocationsAfterFunctionCall  = 0;
};
} // namespace qiti
