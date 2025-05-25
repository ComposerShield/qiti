
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
        virtual ~Listener() noexcept = default;
        /** */
        virtual void onAcquire(const LockData* ld) noexcept = 0;
        /** */
        virtual void onRelease(const LockData* ld) noexcept = 0;
    };
    
    /** Register for lock/unlock notifications. */
    static void addGlobalListener(Listener* listener) noexcept;
    /** Unregister for lock/unlock notifications. */
    static void removeGlobalListener(Listener* listener) noexcept;
    
    /** unique identifier for this lock */
    [[nodiscard]] const void* key() const noexcept { return reinterpret_cast<const void*>(this); }

    /** notify listeners of a lock acquisition */
    void notifyAcquire() noexcept;
    /** notify listeners of a lock release */
    void notifyRelease() noexcept;
};
} // namespace qiti
