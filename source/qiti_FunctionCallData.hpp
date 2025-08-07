
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

#include <memory>
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

    /**
     Returns the CPU time spent inside this function call, in milliseconds.
     
     CPU time is the amount of time the thread actually spent executing
     on the CPU (user + kernel mode), and does ​not​ include time spent off-CPU.
     */
    [[nodiscard]] uint64_t QITI_API getTimeSpentInFunctionCpu_ms() const noexcept;
    
    /**
     Returns the CPU time spent inside this function call, in nanoseconds.
     
     CPU time is the amount of time the thread actually spent executing
     on the CPU (user + kernel mode), and does ​not​ include time spent off-CPU.
     */
    [[nodiscard]] uint64_t QITI_API getTimeSpentInFunctionCpu_ns() const noexcept;
    
    /**
     Returns the wall-clock time spent inside this function call, in milliseconds.
     
     Wall-clock time is the real-world elapsed time between entry and exit,
     and so includes any time the thread was preempted or blocked.
     */
    [[nodiscard]] uint64_t QITI_API getTimeSpentInFunctionWallClock_ms() const noexcept;
    
    /**
     Returns the wall-clock time spent inside this function call, in nanoseconds.
     
     Wall-clock time is the real-world elapsed time between entry and exit,
     and so includes any time the thread was preempted or blocked.
     */
    [[nodiscard]] uint64_t QITI_API getTimeSpentInFunctionWallClock_ns() const noexcept;
    
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
    std::unique_ptr<Impl> pImpl;
    
    //--------------------------------------------------------------------------
    /** \endcond */
    // Doxygen - End Internal Documentation
    //--------------------------------------------------------------------------
};
} // namespace qiti
