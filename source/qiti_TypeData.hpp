
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
 TODO: Implement
 Abtracts a type and its history of use
 */
class [[deprecated("WIP - Not finished implementing.")]]
TypeData
{
public:
    /** */
    [[nodiscard]] const char* QITI_API getTypeName() const noexcept;
    
    //--------------------------------------------------------------------------
    // Doxygen - Begin Internal Documentation
    /** \cond INTERNAL */
    //--------------------------------------------------------------------------
    
    /** */
    QITI_API_INTERNAL TypeData(const void* functionAddress) noexcept;
    /** */
    QITI_API_INTERNAL ~TypeData() noexcept;
    
    struct Impl;
    /** */
    [[nodiscard]] Impl* QITI_API_INTERNAL getImpl() noexcept;
    /** */
    [[nodiscard]] const Impl* QITI_API_INTERNAL getImpl() const noexcept;
    
    /** Move Constructor */
    QITI_API_INTERNAL TypeData(TypeData&& other) noexcept;
    /** Move Operator */
    [[nodiscard]] TypeData& QITI_API_INTERNAL operator=(TypeData&& other) noexcept;
    
private:
    std::unique_ptr<Impl> pImpl;
    
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
