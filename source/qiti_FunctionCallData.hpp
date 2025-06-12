
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

#include "qiti_Utils.hpp"

#include <thread>

//--------------------------------------------------------------------------

namespace qiti
{
//--------------------------------------------------------------------------
/**
 Abtracts a specific call of a specific function
 
 Tracks per‐call metrics such as heap allocations and bytes allocated.
 */
class FunctionCallData
{
public:    
    /** Calculates how many allocations occurred between function entry and exit. */
    [[nodiscard]] uint64_t QITI_API getNumHeapAllocations() const noexcept;
    
    /** Get the total bytes allocated on the heap during this call. */
    [[nodiscard]] uint64_t QITI_API getAmountHeapAllocated() const noexcept;
    
    /** Get the total amount of time spent inside this function call, in milliseconds. */
    [[nodiscard]] uint64_t QITI_API getTimeSpentInFunction_ms() const noexcept;
    
    /** Get the total amount of time spent inside this function call, in nanoseconds. */
    [[nodiscard]] uint64_t QITI_API getTimeSpentInFunction_ns() const noexcept;
    
    /** Get thread that was responsible for this function call. */
    [[nodiscard]] std::thread::id QITI_API getThreadThatCalledFunction() const noexcept;
    
    //--------------------------------------------------------------------------
    // Doxygen - Begin Internal Documentation
    /** \cond INTERNAL */
    //--------------------------------------------------------------------------
    
    /** */
    QITI_API_INTERNAL FunctionCallData() noexcept;
    /** */
    QITI_API ~FunctionCallData() noexcept;
    
    struct Impl;
    /** */
    [[nodiscard]] Impl* QITI_API_INTERNAL getImpl() noexcept;
    /** */
    [[nodiscard]] const Impl* QITI_API_INTERNAL getImpl() const noexcept;
    
    /**
     Reset this call data to its initial state.

     Destroys and reinitializes the internal Impl on the stack.
     */
    void QITI_API_INTERNAL reset() noexcept;
    
    /** Move Constructor */
    QITI_API_INTERNAL FunctionCallData(FunctionCallData&& other) noexcept;
    /** Move Assignment */
    [[nodiscard]] FunctionCallData& QITI_API_INTERNAL operator=(FunctionCallData&& other) noexcept;
    /** Copy Constructor */
    FunctionCallData(const FunctionCallData&) noexcept;
    /** Copy Assignment */
    [[nodiscard]] FunctionCallData operator=(const FunctionCallData&) noexcept;
    
private:
    // Stack-based pimpl idiom
    static constexpr std::size_t ImplSize  = 128;
    static constexpr std::size_t ImplAlign =  8;
    alignas(ImplAlign) unsigned char implStorage[ImplSize];
    
    /** Prevent heap allocating this class (deleted) */
    void* operator new(std::size_t) = delete;
    /** Prevent heap allocating this class (deleted) */
    void* operator new[](std::size_t) = delete;
    
    //--------------------------------------------------------------------------
    /** \endcond */
    // Doxygen - End Internal Documentation
    //--------------------------------------------------------------------------
};
} // namespace qiti
