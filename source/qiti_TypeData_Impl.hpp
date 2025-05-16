
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

#include <typeindex>
#include <typeinfo>

//--------------------------------------------------------------------------

namespace qiti
{
enum class FunctionType
{
    regular,
    constructor,
    destructor
};

struct TypeData::Impl
{
public:
    QITI_API_INTERNAL Impl(const std::type_info& info) : typeInfo(info) {}
    QITI_API_INTERNAL ~Impl() = default;
    
    std::type_index typeInfo;
    
    unsigned long long numTimesConstructed = 0;
    unsigned long long numTimesDestructed  = 0;
};
} // namespace qiti
