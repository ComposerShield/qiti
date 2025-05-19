
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
enum class FunctionType
{
    regular,
    constructor,
    destructor
};

struct FunctionData::Impl
{
public:
    QITI_API_INTERNAL Impl() = default;
    QITI_API_INTERNAL ~Impl() = default;
    
    char functionNameMangled[128];
    char functionNameReal[128];
    void* address = nullptr;
    
    uint numTimesCalled = 0;
    uint averageTimeSpentInFunctionNanoseconds = 0;
    std::unordered_set<std::thread::id> threadsCalledOn{};
    
    FunctionType functionType = FunctionType::regular;
    
    FunctionCallData lastCallData{};
};
} // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
