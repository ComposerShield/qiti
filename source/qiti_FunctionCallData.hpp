
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_FunctionCallData.hpp
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

#include "qiti_utils.hpp"

//--------------------------------------------------------------------------

namespace qiti
{
/**
 Abtracts a specific call of a specific function
 */
class FunctionCallData
{
public:    
    /** */
    [[nodiscard]] uint QITI_API getNumHeapAllocations() const noexcept;
    
    //--------------------------------------------------------------------------
    
    /** Internal */
    QITI_API_INTERNAL FunctionCallData() noexcept;
    /** */
    QITI_API ~FunctionCallData() noexcept;
    
    struct Impl;
    /** Internal */
    [[nodiscard]] Impl* QITI_API_INTERNAL getImpl() noexcept;
    /** Internal */
    [[nodiscard]] const Impl* QITI_API_INTERNAL getImpl() const noexcept;
    
    /** Internal */
    void QITI_API_INTERNAL reset() noexcept;
    
    /** Internal Move Constructor */
    QITI_API_INTERNAL FunctionCallData(FunctionCallData&& other) noexcept;
    /** Internal Move Assignment */
    [[nodiscard]] FunctionCallData& QITI_API_INTERNAL operator=(FunctionCallData&& other) noexcept;
    /** Internal Copy Constructor */
    FunctionCallData(const FunctionCallData&) noexcept;
    /** Internal Copy Assignment */
    [[nodiscard]] FunctionCallData operator=(const FunctionCallData&) noexcept;
    
private:
    // Stack-based pimpl idiom
    static constexpr std::size_t ImplSize  = 128;
    static constexpr std::size_t ImplAlign =  8;
    alignas(ImplAlign) unsigned char implStorage[ImplSize];
    
    // Prevent heap allocating this class
    void* operator new(std::size_t) = delete;
    void* operator new[](std::size_t) = delete;
};
} // namespace qiti
