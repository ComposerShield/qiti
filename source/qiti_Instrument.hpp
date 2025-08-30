
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_Instrument.hpp
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

#include <cassert>
#include <functional>
#include <unordered_map>
#include <utility>

namespace qiti
{
/**
 Tools for injecting custom logic into the application at runtime.

 The instrument namespace provides functions to hook into the runtime flow,
 allowing you to insert callbacks and additional logic as needed.
 */
class Instrument
{
public:
    /**
     Reset all instrumentation counters and state.
     
     Clears any recorded data from previous profiling, returning the
     instrumentation subsystem to its initial state.
     */
    QITI_API static void resetInstrumentation() noexcept;
    
    /**
     Register a callback to be invoked on the next heap allocation.
     
     Schedules the provided function pointer 'heapAllocCallback' to be
     executed immediately after the next heap allocation occurs. This
     facilitates custom handling or assertions around allocation events.
     */
    QITI_API static void onNextHeapAllocation(std::function<void()> heapAllocCallback) noexcept;
    
    /**
     Triggers an assertion failure when the next heap allocation happens.
     
     Shortcut equivalent to onNextHeapAllocation([]{ assert(false); });
     */
    QITI_API static void assertOnNextHeapAllocation() noexcept;
    
    /**
     Register a callback to be invoked on the next call to a specific function.
     
     Schedules the provided callback to be executed immediately after the next
     invocation of the specified function. This enables custom handling or 
     assertions around specific function calls during testing.
     
     @tparam FuncPtr The function pointer to monitor (free function or member function)
     @param callback The callback to execute when the function is called
     */
    template <auto FuncPtr>
    requires isFreeFunction<FuncPtr>
             || isMemberFunction<FuncPtr>
    QITI_API_INLINE static void onNextFunctionCall(std::function<void()> callback) noexcept
    {
        static constexpr const void* functionAddress = []
        {
            if constexpr (isFreeFunction<FuncPtr>)
                return Profile::getFunctionAddress<FuncPtr>();
            else
                return Profile::getMemberFunctionMockAddress<FuncPtr>();
        }();
        
        qiti::Profile::beginProfilingFunction(functionAddress);
        onNextFunctionCallInternal(std::move(callback), functionAddress);
    }
    
    /**
     Triggers an assertion failure when the next call to a specific function happens.
     
     Shortcut equivalent to onNextFunctionCall<FuncPtr>([]{ assert(false); });
     
     @tparam FuncPtr The function pointer to monitor (free function or member function)
     */
    template <auto FuncPtr>
    requires isFreeFunction<FuncPtr>
             || isMemberFunction<FuncPtr>
    QITI_API_INLINE static void assertOnNextFunctionCall() noexcept
    {
        onNextFunctionCall<FuncPtr>([]{ assert(false); });
    }
    
    //--------------------------------------------------------------------------
    // Doxygen - Begin Internal Documentation
    /** \cond INTERNAL */
    //--------------------------------------------------------------------------
    
    /** */
    QITI_API static void onNextFunctionCallInternal(std::function<void()> callback, const void* functionAddress) noexcept;

private:
    friend class InstrumentHooks;
    
    Instrument() = delete;
    ~Instrument() = delete;
    
    /** */
    QITI_API_INTERNAL static void checkAndExecuteFunctionCallCallback(const void* functionAddress) noexcept;
    
    //--------------------------------------------------------------------------
    /** \endcond */
    // Doxygen - End Internal Documentation
    //--------------------------------------------------------------------------
}; // class Instrument
}  // namespace qiti
