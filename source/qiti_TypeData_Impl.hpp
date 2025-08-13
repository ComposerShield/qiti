
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_TypeData_Impl.hpp
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

#include "qiti_TypeData.hpp"

#include <cstdint>
#include <typeindex>
#include <typeinfo>

//--------------------------------------------------------------------------
// Doxygen - Begin Internal Documentation
/** \cond INTERNAL */
//--------------------------------------------------------------------------

namespace qiti
{
struct TypeData::Impl
{
public:
    QITI_API_INTERNAL Impl(const std::type_info& info, const char* name) 
        : typeInfo(info), typeName(name) {}
    QITI_API_INTERNAL ~Impl() = default;
    
    std::type_index typeInfo;
    const char* typeName = nullptr;
    
    // Instance tracking
    uint64_t numConstructions = 0;
    uint64_t numDestructions = 0;
    uint64_t currentLiveInstances = 0;
    uint64_t peakLiveInstances = 0;
    
    // Memory tracking
    uint64_t totalMemoryAllocated = 0;
    uint64_t currentMemoryUsed = 0;
    uint64_t peakMemoryUsed = 0;
};
} // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
