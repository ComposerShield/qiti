
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_TypeData.cpp
 *
 * @author   Adam Shield
 * @date     2025-05-16
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#define _GLIBCXX_EXTERN_TEMPLATE 0

#include "qiti_TypeData.hpp"

#include "qiti_TypeData_Impl.hpp"

#include "qiti_ScopedNoHeapAllocations.hpp"

//--------------------------------------------------------------------------

namespace qiti
{
TypeData::Impl*       TypeData::getImpl()       noexcept { return reinterpret_cast<Impl*>(implStorage); }
const TypeData::Impl* TypeData::getImpl() const noexcept { return reinterpret_cast<const Impl*>(implStorage); }

const char* TypeData::getTypeName() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->typeInfo.name();
}
} // namespace qiti
