
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
    uint64_t getLengthOfTest_ms() const noexcept;
    /** */
    uint64_t getLengthOfTest_ns() const noexcept;
    
private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
//--------------------------------------------------------------------------
} // namespace qiti
//--------------------------------------------------------------------------
