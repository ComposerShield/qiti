
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_profile.hpp
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

#include <array>
#include <cstdint>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>

namespace qiti
{
namespace FunctionNameHelpers
{
    // A constexpr helper that slices out “FuncPtr = …” from __PRETTY_FUNCTION__.
    template <auto FuncPtr>
    consteval auto QITI_API_INTERNAL makeFunctionNameArray() noexcept
    {
        constexpr const char* fullFuncName = __PRETTY_FUNCTION__;
        constexpr std::string_view pretty = fullFuncName;
        
        constexpr std::string_view marker = "FuncPtr = ";
        constexpr std::size_t start = pretty.find(marker) + marker.size();
        constexpr std::size_t end   = pretty.rfind(']');
        static_assert(start != std::string_view::npos, "Could not find “FuncPtr = ”");
        static_assert(end   != std::string_view::npos, "Could not find trailing ‘]’");
        
        constexpr std::string_view raw = pretty.substr(start, end - start);
        constexpr std::string_view funcPtrString =
        (! raw.empty() && raw.front() == '&')
        ? raw.substr(1)
        : raw;
        
        std::array<char, funcPtrString.size() + 1> tmp = {};
        for (std::size_t i = 0; i < funcPtrString.size(); ++i)
        {
            tmp[i] = funcPtrString[i];
        }
        tmp[funcPtrString.size()] = '\0';
        return tmp;
    }
    
    // Each FuncPtr gets exactly one array-of-chars at namespace scope:
    template <auto FuncPtr>
    inline static constexpr auto functionNameArray = makeFunctionNameArray<FuncPtr>();
    
    // Constexpr pointer into that array:
    template <auto FuncPtr>
    inline static constexpr const char* functionNameCStr = functionNameArray<FuncPtr>.data();
} // namespace FunctionNameHelpers

/**
 Utility class for runtime profiling of functions and types.
 
 The Profile class provides a convenient interface for instrumenting and querying profiling
 data at runtime.
 */
class Profile
{
public:
    /** */
    static void QITI_API resetProfiling() noexcept;
    
    /** */
    template<auto FuncPtr>
    requires isFreeFunction<FuncPtr>
    static void QITI_API inline beginProfilingFunction() noexcept
    {
        static constexpr auto functionAddress = getFunctionAddress<FuncPtr>();
        static constexpr auto functionName    = getFunctionName<FuncPtr>();
        beginProfilingFunction(functionAddress, functionName);
    }
    
    /** */
    template<auto FuncPtr>
    requires isMemberFunction<FuncPtr>
    static void QITI_API inline beginProfilingFunction() noexcept
    {
        static constexpr auto functionAddress = getMemberFunctionMockAddress<FuncPtr>();
        static constexpr auto functionName    = getFunctionName<FuncPtr>();
        beginProfilingFunction(functionAddress, functionName);
    }
    
    /** */
    template <auto FuncPtr>
    requires isFreeFunction<FuncPtr>
    static void QITI_API inline endProfilingFunction() noexcept
    {
        endProfilingFunction(reinterpret_cast<const void*>(FuncPtr));
    }
    
    /** */
    template <auto FuncPtr>
    requires isMemberFunction<FuncPtr>
    static void QITI_API inline endProfilingFunction() noexcept
    {
        endProfilingFunction(getMemberFunctionMockAddress<FuncPtr>());
    }
    
    /** */
    [[deprecated("Results in exceptions on Linux")]] static void QITI_API beginProfilingAllFunctions() noexcept;
    
    /** */
    [[deprecated("Results in exceptions on Linux")]] static void QITI_API endProfilingAllFunctions() noexcept;
    
    /** @returns true if we are currently profling function. */
    template<auto FuncPtr>
    requires isFreeFunction<FuncPtr>
    [[nodiscard]] static inline bool QITI_API isProfilingFunction() noexcept
    {
        return isProfilingFunction(reinterpret_cast<const void*>(FuncPtr));
    }
    
    /**
     @returns true if we are currently profling function.
     Member function overload.
     */
    template<auto FuncPtr>
    requires isMemberFunction<FuncPtr>
    [[nodiscard]] static inline bool QITI_API isProfilingFunction() noexcept
    {
        return isProfilingFunction(getMemberFunctionMockAddress<FuncPtr>());
    }
    
    /** */
    template<typename Type>
    static inline void QITI_API beginProfilingType() noexcept { beginProfilingType( typeid(Type) ); }
    
    /** */
    template <typename Type>
    static inline void QITI_API endProfilingType() noexcept { endProfilingType( typeid(Type) ); }
    
    /** */
    [[nodiscard]] static uint64_t QITI_API getNumHeapAllocationsOnCurrentThread() noexcept;
    
    /** */
    [[nodiscard]] static uint64_t QITI_API getAmountHeapAllocatedOnCurrentThread() noexcept;
    
    /** */
    template<auto FuncPtr>
    requires isFreeFunction<FuncPtr>
    || isMemberFunction<FuncPtr>
    [[nodiscard]] static consteval const char* QITI_API getFunctionName() noexcept
    {
        return FunctionNameHelpers::functionNameCStr<FuncPtr>;
    }
    
private:
    //--------------------------------------------------------------------------
    // Doxygen - Begin Internal Documentation
    /** \cond INTERNAL */
    //--------------------------------------------------------------------------
    
    friend class InstrumentHooks;
    friend class FunctionData;
    friend class Utils;

    Profile() = delete;
    ~Profile() = delete;
    
    /** */
    static void QITI_API_INTERNAL updateFunctionDataOnEnter(const void* this_fn) noexcept;
    
    /** */
    static void QITI_API_INTERNAL updateFunctionDataOnExit(const void* this_fn) noexcept;

    /** */
    template <auto FuncPtr>
    requires isFreeFunction<FuncPtr>
    struct FunctionAddressHolder
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
        static constexpr const void* value = (const void*)FuncPtr; // NOLINT
#pragma clang diagnostic pop
    };
    
    /** */
    template <auto FuncPtr>
    requires isMemberFunction<FuncPtr>
    struct MemberFunctionMockAddressHolder
    {
        static constexpr void* value = nullptr;
    };
    
    /**
     Returns a mock "address" we can use for member functions.
     C++ does not (portably) allow function pointers to member functions.
     Addresses of static variables are guaranteed to be unique so we can
     create one for each member function we wish to profile.
     */
    template <auto FuncPtr>
    requires isFreeFunction<FuncPtr>
    [[nodiscard]] static consteval const void* QITI_API getFunctionAddress() noexcept
    {
        return FunctionAddressHolder<FuncPtr>::value;
    }
    
    /**
     Returns a mock "address" we can use for member functions.
     C++ does not (portably) allow function pointers to member functions.
     Addresses of static variables are guaranteed to be unique so we can
     create one for each member function we wish to profile.
     */
    template <auto FuncPtr>
    requires isMemberFunction<FuncPtr>
    [[nodiscard]] static consteval const void* QITI_API getMemberFunctionMockAddress() noexcept
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
        // c-style cast required because reinterpret_cast cannot be performed at compile-time
        return (const void*)(&MemberFunctionMockAddressHolder<FuncPtr>::value); // NOLINT
#pragma clang diagnostic pop
    }
    
    /** */
    static void QITI_API beginProfilingFunction(const void* functionAddress, const char* functionName = nullptr) noexcept;
    
    /** */
    static void QITI_API endProfilingFunction(const void* functionAddress) noexcept;
    
    /** */
    static void QITI_API beginProfilingType(std::type_index functionAddress) noexcept;
    
    /** */
    static void QITI_API endProfilingType(std::type_index functionAddress) noexcept;
    
    /**
     @returns true if we are currently profling function.
     */
    [[nodiscard]] static bool QITI_API isProfilingFunction(const void* funcAddress) noexcept;
}; // class Profile
}  // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
