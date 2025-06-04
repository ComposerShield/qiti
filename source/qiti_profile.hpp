
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
namespace profile
{
/** */
void QITI_API resetProfiling() noexcept;

/**
 \internal
 Returns a mock "address" we can use for member functions.
 C++ does not (portably) allow function pointers to member functions.
 Addresses of static variables are guaranteed to be unique so we can
 create one for each member function we wish to profile.
 */
template <auto FuncPtr>
requires std::is_member_function_pointer_v<decltype(FuncPtr)>
[[nodiscard]] const void* getMemberFunctionMockAddress() noexcept
{
    static constexpr void* uniqueAddress = nullptr;
    return reinterpret_cast<const void*>(&uniqueAddress);
}

/** */
template<auto FuncPtr>
requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
         || std::is_member_function_pointer_v<decltype(FuncPtr)>
[[nodiscard]] const char* QITI_API getFunctionName() noexcept
{
    // Grab the full pretty‐function string:
    constexpr const char* fullFuncName = __PRETTY_FUNCTION__;
    constexpr std::string_view pretty = fullFuncName;

    // Find “FuncPtr = ” and the closing ‘]’:
    constexpr std::string_view marker = "FuncPtr = ";
    constexpr std::size_t start = pretty.find(marker) + marker.size();
    constexpr std::size_t end   = pretty.rfind(']');
    static_assert(start != std::string_view::npos, "Could not find ‘FuncPtr = ’");
    static_assert(end   != std::string_view::npos, "Could not find trailing ‘]’");

    // Slice out exactly the text between “FuncPtr = ” and ‘]’:
    constexpr std::string_view raw = pretty.substr(start, end - start);
    //    e.g. raw == "&qiti::example::profile::TestType::testFunc"

    // Drop a leading '&' if present:
    constexpr std::string_view funcPtrString =
        (!raw.empty() && raw.front() == '&')
            ? raw.substr(1)    // skip the first character
            : raw;             // otherwise keep the whole thing
    
    // Copy contents into a char array, marked static so it can be shared outside this function
    static const auto funcNameArray = [&funcPtrString] () constexpr
    {
        // sv.size() is constexpr ⇒ this makes a std::array<char, N+1>.
        std::array<char, funcPtrString.size() + 1> tmp = {};
        for (std::size_t i = 0; i < funcPtrString.size(); ++i)
        {
            tmp[i] = funcPtrString[i];
        }
        tmp[funcPtrString.size()] = '\0';  // null-terminate
        return tmp;             // returns a std::array<char, N+1>
    }();
    
    static const char* const cstr = funcNameArray.data();
    
    return cstr;
}

/** \internal */
void QITI_API beginProfilingFunction(const void* functionAddress, const char* functionName = nullptr) noexcept;

/** \internal */
void QITI_API endProfilingFunction(const void* functionAddress) noexcept;

/** */
template<auto FuncPtr>
requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
void QITI_API inline beginProfilingFunction() noexcept
{
    static const auto* functionAddress = reinterpret_cast<const void*>(FuncPtr);
    static const char* functionName    = getFunctionName<FuncPtr>();
    beginProfilingFunction(functionAddress, functionName);
}

/** */
template<auto FuncPtr>
requires std::is_member_function_pointer_v<decltype(FuncPtr)>
void QITI_API inline beginProfilingFunction() noexcept
{
    static const auto* functionAddress = getMemberFunctionMockAddress<FuncPtr>();
    static const char* functionName    = getFunctionName<FuncPtr>();
    beginProfilingFunction(functionAddress, functionName);
}

/** */
template <auto FuncPtr>
requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
void QITI_API inline endProfilingFunction() noexcept
{
    endProfilingFunction(reinterpret_cast<const void*>(FuncPtr));
}

/** */
template <auto FuncPtr>
requires std::is_member_function_pointer_v<decltype(FuncPtr)>
void QITI_API inline endProfilingFunction() noexcept
{
    endProfilingFunction(getMemberFunctionMockAddress<FuncPtr>());
}

/** */
[[deprecated("Results in exceptions on Linux")]] void QITI_API beginProfilingAllFunctions() noexcept;

/** */
[[deprecated("Results in exceptions on Linux")]] void QITI_API endProfilingAllFunctions() noexcept;

/**
 \internal
 @returns true if we are currently profling function.
 Free function overload.
 */
[[nodiscard]] bool QITI_API isProfilingFunction(const void* funcAddress) noexcept;

/** @returns true if we are currently profling function. */
template<auto FuncPtr>
requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
[[nodiscard]] inline bool QITI_API isProfilingFunction() noexcept
{
    return isProfilingFunction(reinterpret_cast<const void*>(FuncPtr));
}

/**
 @returns true if we are currently profling function.
 Member function overload.
 */
template<auto FuncPtr>
requires std::is_member_function_pointer_v<decltype(FuncPtr)>
[[nodiscard]] inline bool QITI_API isProfilingFunction() noexcept
{
    return isProfilingFunction(getMemberFunctionMockAddress<FuncPtr>());
}

/** \internal */
void QITI_API beginProfilingType(std::type_index functionAddress) noexcept;

/** \internal */
void QITI_API endProfilingType(std::type_index functionAddress) noexcept;

/** */
template<typename Type>
inline void QITI_API beginProfilingType() noexcept { beginProfilingType( typeid(Type) ); }

/** */
template <typename Type>
inline void QITI_API endProfilingType() noexcept { endProfilingType( typeid(Type) ); }

/** */
[[nodiscard]] uint64_t QITI_API getNumHeapAllocationsOnCurrentThread() noexcept;

/** */
[[nodiscard]] uint64_t QITI_API getAmountHeapAllocatedOnCurrentThread() noexcept;

/** \internal */
void QITI_API_INTERNAL updateFunctionDataOnEnter(const void* this_fn) noexcept;

/** \internal */
void QITI_API_INTERNAL updateFunctionDataOnExit(const void* this_fn) noexcept;

} // namespace profile
} // namespace qiti
