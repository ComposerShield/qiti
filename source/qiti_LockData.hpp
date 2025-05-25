
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

namespace qiti
{
/**
 */
class LockData
{
public:
    struct Listener
    {
        QITI_API Listener() noexcept = default;
        virtual QITI_API ~Listener() noexcept = default;
        /** User provided callback. */
        virtual void QITI_API onAcquire(const LockData* ld) noexcept = 0;
        /** User provided callback. */
        virtual void QITI_API onRelease(const LockData* ld) noexcept = 0;
    };
    
    /** Register for lock/unlock notifications. */
    static void QITI_API addGlobalListener(Listener* listener) noexcept;
    /** Unregister for lock/unlock notifications. */
    static void QITI_API removeGlobalListener(Listener* listener) noexcept;
    
    /** Unique identifier for this lock */
    [[nodiscard]] const void* QITI_API key() const noexcept { return reinterpret_cast<const void*>(this); }

    /** Notify listeners of a lock acquisition */
    void QITI_API notifyAcquire() noexcept;
    /** Notify listeners of a lock release */
    void QITI_API notifyRelease() noexcept;
};
} // namespace qiti
