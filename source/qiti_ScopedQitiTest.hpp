
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
 */
class ScopedQitiTest
{
public:
    /** */
    QITI_API ScopedQitiTest() noexcept;
    /** */
    QITI_API ~ScopedQitiTest() noexcept;
    
    /** */
    uint64_t QITI_API getLengthOfTest_ms() const noexcept;
    /** */
    uint64_t QITI_API getLengthOfTest_ns() const noexcept;
    
    /** 50 ms by default.*/
    void QITI_API setMaximumDurationOfTest_ms(uint64_t ms) noexcept;
    /** */
    void QITI_API permitLongTest() noexcept;
    
private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
//--------------------------------------------------------------------------
} // namespace qiti
//--------------------------------------------------------------------------
