
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
 
 To obtain a FunctionCallData object, call FunctionData::getLastFunctionCall()
 on the FunctionData instance for the function you want to analyze.
 
 Note: Qiti currently only stores data for the most recent function call,
 not historical data for all previous calls.
 */
class FunctionCallData
{
public:    
    /** Calculates how many allocations occurred between function entry and exit. */
    [[nodiscard]] QITI_API uint64_t getNumHeapAllocations() const noexcept;
    
    /** Get the total bytes allocated on the heap during this call. */
    [[nodiscard]] QITI_API uint64_t getAmountHeapAllocated() const noexcept;

    /**
     Returns the CPU time spent inside this function call, in milliseconds.
     
     CPU time is the amount of time the thread actually spent executing
     on the CPU (user + kernel mode), and does ​not​ include time spent off-CPU.
     
     Feature is not supported on Windows.
     */
#ifdef _WIN32
    [[deprecated("Feature is not supported on Windows")]]
#endif
    [[nodiscard]] QITI_API uint64_t getTimeSpentInFunctionCpu_ms() const noexcept;
    
    /**
     Returns the CPU time spent inside this function call, in nanoseconds.
     
     CPU time is the amount of time the thread actually spent executing
     on the CPU (user + kernel mode), and does ​not​ include time spent off-CPU.
     
     Feature is not supported on Windows.
     */
#ifdef _WIN32
    [[deprecated("Feature is not supported on Windows")]]
#endif
    [[nodiscard]] QITI_API uint64_t getTimeSpentInFunctionCpu_ns() const noexcept;
    
    /**
     Returns the wall-clock time spent inside this function call, in milliseconds.
     
     Wall-clock time is the real-world elapsed time between entry and exit,
     and so includes any time the thread was preempted or blocked.
     */
    [[nodiscard]] QITI_API uint64_t getTimeSpentInFunctionWallClock_ms() const noexcept;
    
    /**
     Returns the wall-clock time spent inside this function call, in nanoseconds.
     
     Wall-clock time is the real-world elapsed time between entry and exit,
     and so includes any time the thread was preempted or blocked.
     */
    [[nodiscard]] QITI_API uint64_t getTimeSpentInFunctionWallClock_ns() const noexcept;
    
    /** Get thread that was responsible for this function call. */
    [[nodiscard]] QITI_API std::thread::id getThreadThatCalledFunction() const noexcept;
    
    /** 
     Get the function that called this function.
     
     @returns A pointer to the FunctionData of the calling function, or nullptr if
     this function was called from outside the profiled call stack (e.g. from main
     or from a function not being profiled by Qiti).
     
     Note: This only works reliably when you have called ScopedQitiTest::enableProfilingOnAllFunctions(true).
     */
    [[nodiscard]] QITI_API const FunctionData* getCaller() const noexcept;
    
    /**
     @returns True if this function call threw an exception during execution.
     
     Indicates whether this specific function executed a throw statement during
     this call. This does not include exceptions thrown by other functions that
     this function called.
     */
    [[nodiscard]] QITI_API bool didThrowException() const noexcept;
    
    /**
     @returns The total number of exceptions thrown during this specific function call.
     
     Counts the number of times this specific function executed a throw statement
     during this particular call. This does not include exceptions thrown by other 
     profiled functions that this function called. A function can potentially throw multiple
     exceptions during a single call (e.g., in loops or exception handling code).
     @returns 0 if no exceptions were thrown during this call.
     */
    [[nodiscard]] QITI_API uint64_t getNumExceptionsThrown() const noexcept;
    
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
    [[nodiscard]] QITI_API_INTERNAL Impl* getImpl() noexcept;
    /** */
    [[nodiscard]] QITI_API_INTERNAL const Impl* getImpl() const noexcept;
    
    /**
     Reset this call data to its initial state.

     Destroys and reinitializes the internal Impl on the stack.
     */
    QITI_API_INTERNAL void reset() noexcept;
    
    /** Move Constructor */
    QITI_API_INTERNAL FunctionCallData(FunctionCallData&& other) noexcept;
    /** Move Assignment */
    [[nodiscard]] QITI_API_INTERNAL FunctionCallData& operator=(FunctionCallData&& other) noexcept;
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
