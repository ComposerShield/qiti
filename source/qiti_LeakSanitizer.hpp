
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_LeakSanitizer.hpp
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

#include <atomic>
#include <functional>
#include <string>

//--------------------------------------------------------------------------
// Doxygen - Begin Internal Documentation
/** \cond INTERNAL */
//--------------------------------------------------------------------------

namespace qiti
{
//--------------------------------------------------------------------------
/**
 Memory leak detector that tracks heap allocations during function execution.
 
 LeakSanitizer monitors heap allocations and deallocations during the execution
 of a provided function to detect memory leaks. It compares the amount of heap
 memory allocated before and after the function runs - if they don't match,
 it indicates a memory leak.
 
 @note This class works by leveraging Qiti's malloc hooks to track allocations
 on the current thread. It will only detect leaks from allocations made through
 operator new/delete (macOS) or malloc/free (Linux) that are instrumented by Qiti.
 
 @code{.cpp}
 qiti::LeakSanitizer lsan;
 lsan.run([]() {
     int* ptr = new int(42);
     delete ptr; // No leak
 });
 assert(lsan.passed());
 
 lsan.run([]() {
     int* ptr = new int(42);
     // Forgot to delete - leak detected!
 });
 assert(lsan.failed());
 @endcode
 */
class
#if ! defined(__APPLE__) // LSAN only supported on MacOS, TODO: support Linux
[[deprecated("LeakSanitizer not yet supported on Linux")]]
#endif
LeakSanitizer final
{
public:
    /** Default constructor. Initializes leak sanitizer in passed state. */
    QITI_API LeakSanitizer() noexcept;
    
    /** Destructor. */
    QITI_API ~LeakSanitizer() noexcept;
    
    /** 
     Execute function and check for memory leaks.
     
     Captures the current heap allocation amount, runs the provided function,
     then checks if the heap allocation amount matches. If there's a difference,
     it indicates a memory leak and marks the test as failed.
     
     @param func Function to execute and monitor for leaks. Can be nullptr (no-op).
     */
    void QITI_API run(std::function<void()> func) noexcept;
    
    /** 
     Check if all leak detection tests passed.
     @return true if no leaks detected, false if any leaks found
     */
    [[nodiscard]] bool QITI_API passed() const noexcept;
    
    /** 
     Check if any leak detection tests failed.
     @return true if any leaks detected, false if all tests passed
     */
    [[nodiscard]] inline bool QITI_API failed() const noexcept { return ! passed(); }
    
    /**
     Get detailed report of memory allocation during testing.
     @return String containing allocation/deallocation details and any detected leaks
     */
    [[nodiscard]] std::string QITI_API getReport() const noexcept;

    /** Move Constructor */
    QITI_API LeakSanitizer(LeakSanitizer&& other) noexcept;
    /** Move Assignment */
    LeakSanitizer& QITI_API operator=(LeakSanitizer&& other) noexcept;
    
private:
    //--------------------------------------------------------------------------
    // Doxygen - Begin Internal Documentation
    /** \cond INTERNAL */
    //--------------------------------------------------------------------------
    
    std::atomic<bool> _passed = true;
    uint64_t _totalAllocated = 0;
    uint64_t _totalDeallocated = 0;
    uint64_t _netLeak = 0;
    
    /** Copy Constructor (deleted) */
    LeakSanitizer(const LeakSanitizer&) = delete;
    /** Copy Assignment (deleted) */
    LeakSanitizer& operator=(const LeakSanitizer&) = delete;
    
    //--------------------------------------------------------------------------
    /** \endcond */
    // Doxygen - End Internal Documentation
    //--------------------------------------------------------------------------
};
} // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
