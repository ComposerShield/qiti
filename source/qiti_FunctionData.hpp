
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
/**
 Abtracts a function and its history of use
 */
class FunctionData
{
public:
    /** */
    template <auto FuncPtr>
    requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
    [[nodiscard]] static const qiti::FunctionData* QITI_API getFunctionData() noexcept
    {
        return getFunctionDataMutable<FuncPtr>(); // wrap in const
    }
    
    /** */
    [[nodiscard]] const char* QITI_API getFunctionName() const noexcept;
    
    /** */
    [[nodiscard]] const char* QITI_API getMangledFunctionName() const noexcept;
    
    /** */
    [[nodiscard]] uint64_t QITI_API getNumTimesCalled() const noexcept;
    
    /**
     Returns true if function was called on the provided thread.
     
     Call example:
     
     */
    [[nodiscard]] bool QITI_API wasCalledOnThread(std::thread::id thread) const noexcept;

    /** */
    [[nodiscard]] FunctionCallData QITI_API getLastFunctionCall() const noexcept;
    
    //--------------------------------------------------------------------------
    // Doxygen - Begin Internal Documentation
    /** \cond INTERNAL */
    //--------------------------------------------------------------------------
    
    /** */
    QITI_API_INTERNAL FunctionData(void* functionAddress) noexcept;
    /** */
    QITI_API_INTERNAL ~FunctionData() noexcept;
    
    /** */
    template <auto FuncPtr>
    requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
    [[nodiscard]] static qiti::FunctionData* QITI_API_INTERNAL getFunctionDataMutable() noexcept
    {
        qiti::profile::beginProfilingFunction<FuncPtr>();
        return &qiti::getFunctionDataFromAddress(reinterpret_cast<void*>(FuncPtr));
    }
    
    /** */
    void QITI_API_INTERNAL functionCalled() noexcept;
    
    struct Impl;
    /** */
    [[nodiscard]] Impl* QITI_API_INTERNAL getImpl() noexcept;
    /** */
    [[nodiscard]] const Impl* QITI_API_INTERNAL getImpl() const noexcept;
    
    /** */
    struct Listener
    {
        virtual ~Listener() = default;
        virtual void QITI_API_INTERNAL onFunctionEnter(const FunctionData*) noexcept = 0;
        virtual void QITI_API_INTERNAL onFunctionExit (const FunctionData*) noexcept = 0;
    };
    
    /** */
    void QITI_API_INTERNAL addListener(Listener*) noexcept;
    /** */
    void QITI_API_INTERNAL removeListener(Listener*) noexcept;
    
    /** Move Constructor */
    QITI_API_INTERNAL FunctionData(FunctionData&& other) noexcept;
    /** Move Assignment */
    [[nodiscard]] FunctionData& QITI_API_INTERNAL operator=(FunctionData&& other) noexcept;
    
private:
    // Stack-based pimpl idiom
    static constexpr std::size_t ImplSize  = 488;
    static constexpr std::size_t ImplAlign =  8;
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
