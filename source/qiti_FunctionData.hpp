
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_FunctionData.hpp
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

#include "qiti_FunctionCallData.hpp"
#include "qiti_profile.hpp"
#include "qiti_utils.hpp"

#include <cstdint>
#include <thread>

//--------------------------------------------------------------------------

namespace qiti
{
//--------------------------------------------------------------------------
/**
 Abtracts a function and its history of use
 */
class FunctionData
{
public:
    /**
     Begins profiling for FuncPtr and returns a pointer to the corresponding FunctionData instance.
     */
    template <auto FuncPtr>
    requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
    [[nodiscard]] static const qiti::FunctionData* QITI_API getFunctionData() noexcept
    {
        return getFunctionDataMutable<FuncPtr>(); // wrap in const
    }
    
    /**
     Get the demangled name of the function.

     Returns a human-readable name for the function without mangling.
     */
    [[nodiscard]] const char* QITI_API getFunctionName() const noexcept;
    
    /**
     Get the mangled name of the function.

     Returns the compiler-generated mangled identifier for the function.
     */
    [[nodiscard]] const char* QITI_API getMangledFunctionName() const noexcept;
    
    /**
     Get the total number of times this function was called.

     Returns the count of all recorded invocations that have occurred since we began profiling the function.
     */
    [[nodiscard]] uint64_t QITI_API getNumTimesCalled() const noexcept;
    
    /**
     Check if the function was called on a specific thread.

     Returns true if any recorded invocation occurred on the thread identified by 'thread'.
     */
    [[nodiscard]] bool QITI_API wasCalledOnThread(std::thread::id thread) const noexcept;

    /**
     Retrieve the most recent function call data.

     Returns a FunctionCallData object representing the last recorded invocation of this function.
     */
    [[nodiscard]] FunctionCallData QITI_API getLastFunctionCall() const noexcept;
    
    //--------------------------------------------------------------------------
    // Doxygen - Begin Internal Documentation
    /** \cond INTERNAL */
    //--------------------------------------------------------------------------
    
    /**
     Construct FunctionData for a raw function address.

     Initializes internal tracking structures using the provided function address.
     */
    QITI_API_INTERNAL FunctionData(void* functionAddress) noexcept;
    
    /**
     Destroy this FunctionData instance.

     Cleans up any resources associated with tracking this function.
     */
    QITI_API_INTERNAL ~FunctionData() noexcept;
    
    /** Begins profiling for FuncPtr and returns a pointer to the corresponding mutable FunctionData instance. */
    template <auto FuncPtr>
    requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
    [[nodiscard]] static qiti::FunctionData* QITI_API_INTERNAL getFunctionDataMutable() noexcept
    {
        qiti::profile::beginProfilingFunction<FuncPtr>();
        return &qiti::getFunctionDataFromAddress(reinterpret_cast<void*>(FuncPtr));
    }
    
    /**
     Record that the function was called.

     Updates internal state to log a new invocation, including timestamp and thread information.
     */
    void QITI_API_INTERNAL functionCalled() noexcept;
    
    struct Impl;
    /** */
    [[nodiscard]] Impl* QITI_API_INTERNAL getImpl() noexcept;
    /** */
    [[nodiscard]] const Impl* QITI_API_INTERNAL getImpl() const noexcept;
    
    /**
     Listener interface for function entry and exit events.

     Implement this interface to receive callbacks when a function begins or ends execution.
     */
    struct Listener
    {
        virtual ~Listener() = default;
        /** */
        virtual void QITI_API_INTERNAL onFunctionEnter(const FunctionData*) noexcept = 0;
        /** */
        virtual void QITI_API_INTERNAL onFunctionExit (const FunctionData*) noexcept = 0;
    };
    
    /** */
    void QITI_API_INTERNAL addListener(Listener* listener) noexcept;
    /** */
    void QITI_API_INTERNAL removeListener(Listener* listener) noexcept;
    
    /** Move Constructor */
    QITI_API_INTERNAL FunctionData(FunctionData&& other) noexcept;
    /** Move Assignment */
    [[nodiscard]] FunctionData& QITI_API_INTERNAL operator=(FunctionData&& other) noexcept;
    
private:
    // Stack-based pimpl idiom
    static constexpr std::size_t ImplSize  = 504;
    static constexpr std::size_t ImplAlign = 8;
    alignas(ImplAlign) unsigned char implStorage[ImplSize];
    
    /** Copy Constructor (deleted) */
    FunctionData(const FunctionData&) = delete;
    /** Copy Assignment (deleted) */
    FunctionData& operator=(const FunctionData&) = delete;
    
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
