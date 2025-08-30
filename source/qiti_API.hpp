
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_API.hpp
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

#include <type_traits>

/**
 Marks functions to be excluded from instrumentation.
 
 When applied, this attribute tells the compiler (e.g., GCC or Clang)
 not to insert profiling or instrumentation hooks into the annotated
 functions. It is important that we do not instrument our own
 instrumentation code.
 
 Usage:
 \code
 void QITI_API_INTERNAL myFunc();
 \endcode
 
 Not intended for use in client code.
 */
#ifndef QITI_API_INTERNAL
  #define QITI_API_INTERNAL __attribute__((no_instrument_function))
#endif

/**
 Marks functions to be excluded from instrumentation (same as QITI_API_INTERNAL).
 
 This is intended to be used for inlined functions, most notably templates, which do not
 require the visibility setting in QITI_API below.
 
 Usage:
 \code
 QITI_API_INLINE void myFunc();
 \endcode
 
 Not intended for use in client code.
 */
#ifndef QITI_API_INLINE
  #define QITI_API_INLINE __attribute__((no_instrument_function))
#endif

/**
 Controls symbol export/import and visibility.
 Always includes QITI_API_INTERNAL.
 
 Usage:
 \code
 QITI_API void myFunc();
 \endcode
 
 Not intended for use in client code.
 */
#if defined _WIN32 || defined __CYGWIN__
  #ifdef QITI_DYLIB
    #define QITI_API QITI_API_INTERNAL __declspec(dllexport)
    #define QITI_API_VAR __declspec(dllexport)
  #else
    #define QITI_API QITI_API_INTERNAL __declspec(dllimport)
    #define QITI_API_VAR __declspec(dllimport)
  #endif
#else
  #ifdef QITI_DYLIB
    #define QITI_API QITI_API_INTERNAL __attribute__((visibility("default")))
    #define QITI_API_VAR __attribute__((visibility("default")))
  #else
    #define QITI_API QITI_API_INTERNAL
    #define QITI_API_VAR
  #endif
#endif

/**
  QITI_OVERLOAD(ReturnType, FunctionName, ...):
    - ReturnType    : The return type of the target overload.
    - FunctionName    : The unqualified function name (overloaded).
    - ...     : The comma-separated list of parameter types for the overload you want.
  
  which casts the address of Name to the exact function-pointer type
  ReturnType(*)(__VA_ARGS__), thus disambiguating the overload set.
  
  Example:
    // Two overloads:
    void testFuncOverload(int) noexcept;
    void testFuncOverload(float) noexcept;
  
    // Picks the 'int' version without you writing the cast:
    qiti::profile::beginProfilingFunction<QITI_OVERLOAD(void, testFuncOverload, int)>();
 */
#define QITI_OVERLOAD(ReturnType, FunctionName, ...) \
    static_cast<ReturnType(*)(__VA_ARGS__)>(&FunctionName)

namespace qiti
{
//--------------------------------------------------------------------------
// Doxygen - Begin Internal Documentation
/** \cond INTERNAL */
//--------------------------------------------------------------------------
    template<auto FuncPtr>
    concept isFreeFunction =
        std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>;

    template<auto FuncPtr>
    concept isMemberFunction =
        std::is_member_function_pointer_v<decltype(FuncPtr)>;

    /**
     Check if ThreadSanitizer wrapper functionality is enabled.

     @returns true if QITI_ENABLE_CLANG_THREAD_SANITIZER was defined during compilation
     */
    constexpr bool isThreadSanitizerEnabled() noexcept
    {
#ifdef QITI_ENABLE_CLANG_THREAD_SANITIZER
        return true;
#else
        return false;
#endif
    }
//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
} // namespace qiti
