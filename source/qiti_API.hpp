
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

/**
 \internal
 Marks functions to be excluded from instrumentation.
 
 When applied, this attribute tells the compiler (e.g., GCC or Clang)
 not to insert profiling or instrumentation hooks into the annotated
 functions. This is useful for performance‐critical internal routines
 or to avoid recursive instrumentation in low‐level utilities.
 
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
 \internal
 Controls symbol export/import and visibility.
 Always includes QITI_API_INTERNAL.
 
 Usage:
 \code
 void QITI_API myFunc();
 \endcode
 
 Not intended for use in client code.
 */
#if defined _WIN32 || defined __CYGWIN__
  #ifdef QITI_DYLIB
    #define QITI_API QITI_API_INTERNAL __declspec(dllexport)
  #else
    #define QITI_API QITI_API_INTERNAL __declspec(dllimport)
  #endif
#else
  #ifdef QITI_DYLIB
    #define QITI_API QITI_API_INTERNAL __attribute__((visibility("default")))
  #else
    #define QITI_API QITI_API_INTERNAL
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
