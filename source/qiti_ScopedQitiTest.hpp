
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_ScopedQitiTest.hpp
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
#include <memory>

//--------------------------------------------------------------------------
namespace qiti
{
//--------------------------------------------------------------------------
/**
 Initializes Qiti profiling and other functionality.
 
 This object should be placed at the top of any test that wishes to use Qiti.
 It initializes profiling for the test duration and cleans up state when it goes out of scope.
 Can be queried for details regarding the current Qiti-injected unit test.
 
 @code
 TEST_CASE("My Test") {
     qiti::ScopedQitiTest test;
     
     // Your test code here
     auto funcData = qiti::FunctionData::getFunctionData<&myFunction>();
     myFunction();
     auto lastCall = funcData->getLastFunctionCall();
     // ... assertions
 }
 @endcode
 */
class ScopedQitiTest final
{
public:
    /**
     Initialize Qiti profiling for the duration of this scope.
     
     Sets up instrumentation, profiling hooks, and timing measurements.
     Test timing begins when this constructor is called.
     */
    QITI_API ScopedQitiTest() noexcept;
    
    /**
     Clean up Qiti profiling state and finalize measurements.
     
     Validates maximum test duration if one was set.
     */
    QITI_API ~ScopedQitiTest() noexcept;
    
    /**
     Reset all profiling and instrumentation data.
     Optionally reset start time of the test.
     */
    QITI_API void reset(bool resetTestStartTime) noexcept;
    
    /**
     Enable or disable profiling on all instrumented functions.
     
     When enabled, Qiti will profile every function compiled with
     -finstrument-functions, not just those explicitly registered.
     This is required for reliable caller tracking functionality.
     
     Note: Enabling this feature significantly increases profiling overhead.
     
     @param enable If true, profile all functions; if false, only profile explicitly registered functions
     */
    QITI_API void enableProfilingOnAllFunctions(bool enable) noexcept;
    
    /**
     Get the full version string of Qiti.
     
     @returns A string in the format "major.minor.patch" (e.g., "1.0.0")
     */
    [[nodiscard]] QITI_API static const char* getQitiVersionString() noexcept;
    
    /**
     Get the major version number of Qiti.
     
     @returns The major version component (e.g., 1 from "1.0.0")
     */
    [[nodiscard]] QITI_API static int getQitiVersionMajor() noexcept;
    
    /**
     Get the minor version number of Qiti.
     
     @returns The minor version component (e.g., 0 from "1.0.0")
     */
    [[nodiscard]] QITI_API static int getQitiVersionMinor() noexcept;
    
    /**
     Get the patch version number of Qiti.
     
     @returns The patch version component (e.g., 0 from "1.0.0")
     */
    [[nodiscard]] QITI_API static int getQitiVersionPatch() noexcept;
    
    /** 
     Get elapsed length of time since ScopedQitiTest was initialized.
     
     @returns Elapsed time in milliseconds
     */
    [[nodiscard]] QITI_API uint64_t getLengthOfTest_ms() const noexcept;
    
    /** 
     Get elapsed length of time since ScopedQitiTest was initialized.
     
     @returns Elapsed time in nanoseconds
     */
    [[nodiscard]] QITI_API uint64_t getLengthOfTest_ns() const noexcept;
    
    /** Assert if test exceeds duration specified. */
    QITI_API void setMaximumDurationOfTest_ms(uint64_t ms) noexcept;
    
    // Deleted constructors/destructors
    /** Copy Constructor (deleted) */
    ScopedQitiTest(const ScopedQitiTest&) = delete;
    /** Copy Assignment (deleted) */
    ScopedQitiTest& operator=(const ScopedQitiTest&) = delete;
    /** Move Constructor (deleted) */
    ScopedQitiTest(ScopedQitiTest&& other) noexcept;
    /** Move Assignment (deleted) */
    ScopedQitiTest& operator=(ScopedQitiTest&& other) noexcept;
    
private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
//--------------------------------------------------------------------------
} // namespace qiti
//--------------------------------------------------------------------------
