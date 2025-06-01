
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

//--------------------------------------------------------------------------

namespace qiti
{
//--------------------------------------------------------------------------
/**
 */
class LeakSanitizer
{
public:
    QITI_API LeakSanitizer() noexcept;
    QITI_API ~LeakSanitizer() noexcept;
    
    /** */
    [[nodiscard]] bool QITI_API passed() noexcept;
    
    /** */
    [[nodiscard]] inline bool QITI_API failed() noexcept { return ! passed(); }

    /** Move Constructor */
    QITI_API LeakSanitizer(LeakSanitizer&& other) noexcept;
    /** Move Assignment */
    [[nodiscard]] LeakSanitizer& QITI_API operator=(LeakSanitizer&& other) noexcept;
    
private:
    //--------------------------------------------------------------------------
    // Doxygen - Begin Internal Documentation
    /** \cond INTERNAL */
    //--------------------------------------------------------------------------
    
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
