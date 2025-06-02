
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_ScopedNoHeapAllocations.hpp
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
 
 Cleans up state when it goes out of scope.
 Can be queried for details regarding the current Qiti-injected unit test.
 */
class ScopedQitiTest
{
public:
    /** */
    QITI_API ScopedQitiTest() noexcept;
    /** */
    QITI_API ~ScopedQitiTest() noexcept;
    
    /** Returns elapsed length of time since ScopedQitiTest was initialized. */
    [[nodiscard]] uint64_t QITI_API getLengthOfTest_ms() const noexcept;
    /** Returns elapsed length of time since ScopedQitiTest was initialized. */
    [[nodiscard]] uint64_t QITI_API getLengthOfTest_ns() const noexcept;
    
    /** Assert if test exceeds duration specified. */
    void QITI_API setMaximumDurationOfTest_ms(uint64_t ms) noexcept;
    
private:
    struct Impl;
    std::unique_ptr<Impl> impl;
    
    /** Copy Constructor (deleted) */
    ScopedQitiTest(const ScopedQitiTest&) = delete;
    /** Copy Assignment (deleted) */
    ScopedQitiTest& operator=(const ScopedQitiTest&) = delete;
    /** Move Constructor (deleted) */
    ScopedQitiTest(ScopedQitiTest&& other) noexcept;
    /** Move Assignment (deleted) */
    ScopedQitiTest& operator=(ScopedQitiTest&& other) noexcept;
};
//--------------------------------------------------------------------------
} // namespace qiti
//--------------------------------------------------------------------------
