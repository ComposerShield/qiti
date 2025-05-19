
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

//--------------------------------------------------------------------------
// Doxygen - Begin Internal Documentation
/** \cond INTERNAL */
//--------------------------------------------------------------------------

namespace qiti
{
struct FunctionCallData::Impl
{
    std::chrono::steady_clock::time_point begin_time;
    std::chrono::steady_clock::time_point end_time;
    std::thread::id callingThread;
    
    uint64_t timeSpentInFunctionNanoseconds = 0;
    
    uint64_t numHeapAllocationsBeforeFunctionCall = 0;
    uint64_t numHeapAllocationsAfterFunctionCall  = 0;
};
} // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
