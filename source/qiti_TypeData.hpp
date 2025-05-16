
#pragma once

#include "qiti_API.hpp"

#include <cstdio>

//--------------------------------------------------------------------------

namespace qiti
{
/**
 Abtracts a type and its history of use
 */
class TypeData
{
public:
    /** */
    [[nodiscard]] const char* QITI_API getTypeName() const noexcept;
    
    /** Internal */
    QITI_API_INTERNAL TypeData(void* functionAddress) noexcept;
    /** Internal */
    QITI_API_INTERNAL ~TypeData() noexcept;
    
    struct Impl;
    /** Internal */
    [[nodiscard]] Impl* QITI_API_INTERNAL getImpl() noexcept;
    /** Internal */
    [[nodiscard]] const Impl* QITI_API_INTERNAL getImpl() const noexcept;
    
    /** Internal Move Constructor */
    QITI_API_INTERNAL TypeData(TypeData&& other) noexcept;
    /** Internal Move Operator */
    [[nodiscard]] TypeData& QITI_API_INTERNAL operator=(TypeData&& other) noexcept;
    
private:
    // Stack-based pimpl idiom
    static constexpr size_t ImplSize  = 456;
    static constexpr size_t ImplAlign =  8;
    alignas(ImplAlign) unsigned char implStorage[ImplSize];
    
    /** Copy Constructor (deleted) */
    TypeData(const TypeData&) = delete;
    /** Copy Assignment (deleted) */
    TypeData& operator=(const TypeData&) = delete;
    
    // Prevent heap allocating this class
    void* operator new(size_t) = delete;
    void* operator new[](size_t) = delete;
};
} // namespace qiti
