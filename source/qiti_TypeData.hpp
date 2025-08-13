
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_TypeData.hpp
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

#include "qiti_API.hpp"
#include "qiti_Profile.hpp"

#include <cstdio>
#include <memory>

//--------------------------------------------------------------------------
// Doxygen - Begin Internal Documentation
/** \cond INTERNAL */
//--------------------------------------------------------------------------

namespace qiti
{
//--------------------------------------------------------------------------
/**
 Tracks type-related metrics and usage patterns.
 
 TypeData provides tracking for C++ types, similar to how FunctionData 
 tracks functions. You can monitor allocations, destructions, and other
 type-specific metrics.
 */
class TypeData
{
public:
    /** Get the demangled name of the tracked type */
    [[nodiscard]] QITI_API const char* getTypeName() const noexcept;
    
    /** Get the total number of instances of this type that have been constructed */
    [[nodiscard]] QITI_API uint64_t getNumConstructions() const noexcept;
    
    /** Get the total number of instances of this type that have been destructed */
    [[nodiscard]] QITI_API uint64_t getNumDestructions() const noexcept;
    
    /** Get the current number of live instances of this type */
    [[nodiscard]] QITI_API uint64_t getNumLiveInstances() const noexcept;
    
    /** Get the peak number of live instances that existed at the same time */
    [[nodiscard]] QITI_API uint64_t getPeakLiveInstances() const noexcept;
    
    /** Get the total amount of memory allocated for instances of this type */
    [[nodiscard]] QITI_API uint64_t getTotalMemoryAllocated() const noexcept;
    
    /** Get the current amount of memory used by live instances */
    [[nodiscard]] QITI_API uint64_t getCurrentMemoryUsed() const noexcept;
    
    /** Get the peak amount of memory used by instances of this type */
    [[nodiscard]] QITI_API uint64_t getPeakMemoryUsed() const noexcept;
    
    /** Record a construction of this type with optional size tracking */
    QITI_API void recordConstruction(size_t instanceSize = 0) noexcept;
    
    /** Record a destruction of this type */
    QITI_API void recordDestruction(size_t instanceSize = 0) noexcept;
    
    /** Reset all tracking data for this type */
    QITI_API void reset() noexcept;
    
    /**
     Template function to get TypeData for a specific type T.
     Similar to FunctionData::getFunctionData<FuncPtr>().
     
     Usage:
     auto* typeData = TypeData::getTypeData<MyClass>();
     */
    template<typename T>
    [[nodiscard]] QITI_API_INLINE static TypeData* getTypeData() noexcept
    {
        static constexpr auto typeName = qiti::Profile::getTypeName<T>();
        return getTypeDataInternal(typeid(T), typeName);
    }
    
    /**
     Template function to get mutable TypeData for a specific type T.
     Begins tracking for the type if not already tracked.
     */
    template<typename T>
    [[nodiscard]] QITI_API_INLINE static TypeData* getTypeDataMutable() noexcept
    {
        qiti::Profile::beginProfilingType<T>();
        return getTypeData<T>();
    }
    
    //--------------------------------------------------------------------------
    // Doxygen - Begin Internal Documentation
    /** \cond INTERNAL */
    //--------------------------------------------------------------------------
    
    /** Internal constructor - use getTypeData<T>() instead */
    QITI_API_INTERNAL TypeData(const std::type_info& typeInfo, const char* typeName) noexcept;
    
    /** Destructor */
    QITI_API_INTERNAL ~TypeData() noexcept;
    
    struct Impl;
    /** Get internal implementation */
    [[nodiscard]] QITI_API_INTERNAL Impl* getImpl() noexcept;
    /** Get internal implementation (const) */
    [[nodiscard]] QITI_API_INTERNAL const Impl* getImpl() const noexcept;
    
    /** Move Constructor */
    QITI_API_INTERNAL TypeData(TypeData&& other) noexcept;
    /** Move Assignment */
    [[nodiscard]] QITI_API_INTERNAL TypeData& operator=(TypeData&& other) noexcept;
    
private:
    std::unique_ptr<Impl> pImpl;
    
    /** Internal implementation for template getTypeData */
    QITI_API static TypeData* getTypeDataInternal(const std::type_info& typeInfo, const char* typeName) noexcept;
    
    /** Copy Constructor (deleted) */
    TypeData(const TypeData&) = delete;
    /** Copy Assignment (deleted) */
    TypeData& operator=(const TypeData&) = delete;
    
    //--------------------------------------------------------------------------
    /** \endcond */
    // Doxygen - End Internal Documentation
    //--------------------------------------------------------------------------
};
} // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
