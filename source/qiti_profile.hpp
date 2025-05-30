
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

//--------------------------------------------------------------------------
// Doxygen - Begin Internal Documentation
/** \cond INTERNAL */
//--------------------------------------------------------------------------

namespace qiti
{
namespace profile
{
/** */
void QITI_API resetProfiling() noexcept;

/** \internal */
void QITI_API beginProfilingFunction(void* functionAddress) noexcept;

/** \internal */
void QITI_API endProfilingFunction(void* functionAddress) noexcept;

/** */
template<auto FuncPtr>
requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
void QITI_API inline beginProfilingFunction() noexcept { beginProfilingFunction( reinterpret_cast<void*>(FuncPtr)); }

/** */
template <auto FuncPtr>
requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
void QITI_API inline endProfilingFunction() noexcept { endProfilingFunction( reinterpret_cast<void*>(FuncPtr)); }

/** */
[[deprecated("Results in exceptions on Linux")]] void QITI_API beginProfilingAllFunctions() noexcept;

/** */
[[deprecated("Results in exceptions on Linux")]] void QITI_API endProfilingAllFunctions() noexcept;

/** */
[[nodiscard]] bool QITI_API isProfilingFunction(void* funcAddress) noexcept;

/** */
template<auto FuncPtr>
requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
[[nodiscard]] inline bool QITI_API isProfilingFunction() noexcept
{
    return isProfilingFunction( reinterpret_cast<void*>(FuncPtr));
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
void QITI_API_INTERNAL updateFunctionDataOnEnter(void* this_fn) noexcept;

/** \internal */
void QITI_API_INTERNAL updateFunctionDataOnExit(void* this_fn) noexcept;

} // namespace profile
} // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
