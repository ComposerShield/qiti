
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_FunctionData_Impl.hpp
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

#include "qiti_FunctionData.hpp"

#include <bitset>
#include <cstdint>
#include <string>
#include <thread>
#include <unordered_set>

//--------------------------------------------------------------------------
// Doxygen - Begin Internal Documentation
/** \cond INTERNAL */
//--------------------------------------------------------------------------

namespace qiti
{
struct FunctionData::Impl
{
public:
    static constexpr const char* unknownFunctionName = "<unknown>";
    
    const char* functionName = unknownFunctionName;
    const void* address = nullptr;
    
    uint64_t numTimesCalled = 0;
    uint64_t averageTimeSpentInFunctionNanoseconds = 0;
    
    static constexpr size_t MAX_THREADS = 256;
    std::bitset<MAX_THREADS> threadsCalledOn;
    
    FunctionType functionType = FunctionType::regular;
    
    std::unordered_set<FunctionData::Listener*> listeners{};
    
    FunctionCallData lastCallData{};
};
} // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------

static_assert(sizeof(std::thread::id) == 8, "goose");
