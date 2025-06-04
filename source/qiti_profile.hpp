
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

#include <cstdint>
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
    static const void* const uniqueAddress = nullptr;
    return reinterpret_cast<const void*>(&uniqueAddress);
}

/** \internal */
void QITI_API beginProfilingFunction(const void* functionAddress) noexcept;

/** \internal */
void QITI_API endProfilingFunction(const void* functionAddress) noexcept;

/** */
template<auto FuncPtr>
requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
void QITI_API inline beginProfilingFunction() noexcept
{
    beginProfilingFunction(reinterpret_cast<const void*>(FuncPtr));
}

/** */
template<auto FuncPtr>
requires std::is_member_function_pointer_v<decltype(FuncPtr)>
void QITI_API inline beginProfilingFunction() noexcept
{
    beginProfilingFunction(getMemberFunctionMockAddress<FuncPtr>());
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
