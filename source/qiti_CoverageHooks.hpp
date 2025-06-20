
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_LockData.hpp
 *
 * @author   Adam Shield
 * @date     2025-05-25
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#pragma once

#include "qiti_API.hpp"

//--------------------------------------------------------------------------
// Doxygen - Begin Internal Documentation
/** \cond INTERNAL */
//--------------------------------------------------------------------------

namespace qiti
{
//--------------------------------------------------------------------------
/**
 */
class CoverageHooks
{
public:
    /** */
    static void QITI_API dump() noexcept;
    /** */
    static void QITI_API reset() noexcept(false);
    
private:
    CoverageHooks() = delete;
    ~CoverageHooks() = delete;
};
} // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
