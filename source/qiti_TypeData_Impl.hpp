
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
    QITI_API_INTERNAL Impl(const std::type_info& info) : typeInfo(info) {}
    QITI_API_INTERNAL ~Impl() = default;
    
    std::type_index typeInfo;
    
    uint64_t numTimesConstructed = 0;
    uint64_t numTimesDestructed  = 0;
};
} // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
