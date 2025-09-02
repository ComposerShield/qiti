
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
class [[deprecated("TypeData implementation is still a work-in-progress.")]] // TODO: remove
TypeData
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
    
    /** Get the size of the tracked type (compile-time) */
    [[nodiscard]] QITI_API size_t getTypeSize() const noexcept;
    
    /**
     Template function to get TypeData for a specific type T.
     Similar to FunctionData::getFunctionData<FuncPtr>().
     
     @note Only types with user-defined constructors can be profiled.
           This excludes trivial types and aggregates that rely solely
           on compiler-generated constructors.
     
     Usage:
     auto* typeData = TypeData::getTypeData<MyClass>();
     
     @tparam T Type to get data for. Must satisfy hasUserDefinedConstructor concept.
     */
    template<typename Type>
    requires hasUserDefinedConstructor<Type>
    [[nodiscard]] QITI_API_INLINE static const TypeData* getTypeData() noexcept
    {
        return getTypeDataMutable<Type>(); // wrap in const
    }
    
    /**
     Template function to get mutable TypeData for a specific type T.
     Begins tracking for the type if not already tracked.
     
     @note Only types with user-defined constructors can be profiled.
           This excludes trivial types and aggregates that rely solely
           on compiler-generated constructors.
     
     @tparam T Type to get data for. Must satisfy hasUserDefinedConstructor concept.
     */
    template<typename Type>
    requires hasUserDefinedConstructor<Type>
    [[nodiscard]] QITI_API_INLINE static TypeData* getTypeDataMutable() noexcept
    {
        static constexpr auto typeName = qiti::Profile::getTypeName<Type>();
        qiti::Profile::beginProfilingType<Type>();
        return getTypeDataInternal(typeid(Type), typeName, sizeof(Type));
    }
    
    //--------------------------------------------------------------------------
    // Doxygen - Begin Internal Documentation
    /** \cond INTERNAL */
    //--------------------------------------------------------------------------
    
    /** Record a construction of this type */
    QITI_API void recordConstruction() noexcept;
    
    /** Record a destruction of this type */
    QITI_API void recordDestruction() noexcept;
    
    /** Reset all tracking data for this type */
    QITI_API void reset() noexcept;
    
    /** Internal constructor - use getTypeData<T>() instead */
    QITI_API_INTERNAL TypeData(const std::type_info& typeInfo,
                               const char* typeName,
                               size_t typeSize) noexcept;
    
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
    
    // Deleted constructors/destructors
    /** Copy Constructor (deleted) */
    TypeData(const TypeData&) = delete;
    /** Copy Assignment (deleted) */
    TypeData& operator=(const TypeData&) = delete;
    
private:
    std::unique_ptr<Impl> pImpl;
    
    /** Internal implementation for template getTypeData */
    QITI_API static TypeData* getTypeDataInternal(const std::type_info& typeInfo,
                                                  const char* typeName,
                                                  size_t typeSize) noexcept;
    
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
