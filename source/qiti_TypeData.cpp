
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

#include "qiti_TypeData.hpp"

#include "qiti_TypeData_Impl.hpp"
#include "qiti_ScopedNoHeapAllocations.hpp"

#include <memory>
#include <utility>

//--------------------------------------------------------------------------

namespace qiti
{
TypeData::Impl*       TypeData::getImpl()       noexcept { return pImpl.get(); }
const TypeData::Impl* TypeData::getImpl() const noexcept { return pImpl.get(); }

TypeData::TypeData(const void* functionAddress) noexcept
    : pImpl(std::make_unique<Impl>(typeid(void))) // TODO: Implement proper type detection
{
    qiti::ScopedNoHeapAllocations noAlloc;
    (void)functionAddress; // unused parameter for now
}

TypeData::~TypeData() noexcept = default;

TypeData::TypeData(TypeData&& other) noexcept
    : pImpl(std::move(other.pImpl))
{
    qiti::ScopedNoHeapAllocations noAlloc;
}

TypeData& TypeData::operator=(TypeData&& other) noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    if (this != &other)
    {
        pImpl = std::move(other.pImpl);
    }
    return *this;
}

const char* TypeData::getTypeName() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->typeInfo.name();
}
} // namespace qiti
