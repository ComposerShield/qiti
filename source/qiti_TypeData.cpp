
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

#include "qiti_MallocHooks.hpp"
#include "qiti_TypeData_Impl.hpp"
#include "qiti_ScopedNoHeapAllocations.hpp"
#include "qiti_Utils.hpp"
#include "qiti_Profile.hpp"

#include <memory>
#include <utility>
#include <unordered_map>
#include <typeindex>
#include <algorithm>

//--------------------------------------------------------------------------

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated" // TODO: remove when TypeData is no longer deprecated
// Global registry for TypeData instances
static std::unordered_map<std::type_index, std::unique_ptr<qiti::TypeData>> g_typeDataRegistry;
#pragma clang diagnostic pop

//--------------------------------------------------------------------------

namespace qiti
{
TypeData::Impl*       TypeData::getImpl()       noexcept { return pImpl.get(); }
const TypeData::Impl* TypeData::getImpl() const noexcept { return pImpl.get(); }

TypeData::TypeData(const std::type_info& typeInfo,
                   const char* typeName,
                   size_t typeSize) noexcept
: pImpl(std::make_unique<Impl>(typeInfo, typeName, typeSize))
{}

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
    return getImpl()->typeName;
}

uint64_t TypeData::getNumConstructions() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    return getImpl()->numConstructions;
}

uint64_t TypeData::getNumDestructions() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    return getImpl()->numDestructions;
}

uint64_t TypeData::getNumLiveInstances() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    return getImpl()->currentLiveInstances;
}

uint64_t TypeData::getPeakLiveInstances() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    return getImpl()->peakLiveInstances;
}

uint64_t TypeData::getTotalMemoryAllocated() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    return getImpl()->totalMemoryAllocated;
}

uint64_t TypeData::getCurrentMemoryUsed() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    return getImpl()->currentMemoryUsed;
}

uint64_t TypeData::getPeakMemoryUsed() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    return getImpl()->peakMemoryUsed;
}

size_t TypeData::getTypeSize() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    // This will be set during getTypeDataInternal call with compile-time sizeof(T)
    return getImpl()->typeSize;
}

void TypeData::recordConstruction() noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    auto* impl = getImpl();
    
    impl->numConstructions++;
    impl->currentLiveInstances++;
    impl->peakLiveInstances = std::max(impl->peakLiveInstances, impl->currentLiveInstances);
    
    size_t instanceSize = impl->typeSize;
    impl->totalMemoryAllocated += instanceSize;
    impl->currentMemoryUsed += instanceSize;
    impl->peakMemoryUsed = std::max(impl->peakMemoryUsed, impl->currentMemoryUsed);
}

void TypeData::recordDestruction() noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    auto* impl = getImpl();
    
    impl->numDestructions++;
    if (impl->currentLiveInstances > 0)
    {
        impl->currentLiveInstances--;
    }
    
    size_t instanceSize = impl->typeSize;
    if (impl->currentMemoryUsed >= instanceSize)
    {
        impl->currentMemoryUsed -= instanceSize;
    }
}

void TypeData::reset() noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    auto* impl = getImpl();
    
    impl->numConstructions = 0;
    impl->numDestructions = 0;
    impl->currentLiveInstances = 0;
    impl->peakLiveInstances = 0;
    impl->totalMemoryAllocated = 0;
    impl->currentMemoryUsed = 0;
    impl->peakMemoryUsed = 0;
}

TypeData* TypeData::getTypeDataInternal(const std::type_info& typeInfo,
                                        const char* typeName,
                                        size_t typeSize) noexcept
{
    qiti::MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;
    
    std::type_index typeIndex(typeInfo);
    
    auto it = g_typeDataRegistry.find(typeIndex);
    if (it != g_typeDataRegistry.end())
    {
        return it->second.get();
    }
    
    // Create new TypeData instance
    auto typeData = std::make_unique<TypeData>(typeInfo, typeName, typeSize);
    TypeData* result = typeData.get();
    g_typeDataRegistry[typeIndex] = std::move(typeData);
    
    return result;
}

} // namespace qiti
