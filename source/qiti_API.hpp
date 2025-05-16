
// Copyright (c) 2025 Adam Shield
// SPDX-License-Identifier: MIT

#pragma once

#ifndef QITI_API_INTERNAL
  #define QITI_API_INTERNAL __attribute__((no_instrument_function))
#endif

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
