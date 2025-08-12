
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

#ifdef _WIN32
#include <windows.h>
// Windows mutex type alias
using pthread_mutex_t = CRITICAL_SECTION;
#else
#include <pthread.h>
#endif

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
class LockData
{
public:
    struct Listener
    {
        QITI_API Listener() noexcept = default;
        virtual QITI_API ~Listener() noexcept = default;
        /** User provided callback. */
        virtual void QITI_API onAcquire(const pthread_mutex_t* ld) noexcept = 0;
        /** User provided callback. */
        virtual void QITI_API onRelease(const pthread_mutex_t* ld) noexcept = 0;
    };
    
    /** Register for lock/unlock notifications. */
    static void QITI_API addGlobalListener(Listener* listener) noexcept;
    /** Unregister for lock/unlock notifications. */
    static void QITI_API removeGlobalListener(Listener* listener) noexcept;
    
    /** Notify listeners of a lock acquisition */
    static void QITI_API notifyAcquire(const pthread_mutex_t* lock) noexcept;
    /** Notify listeners of a lock release */
    static void QITI_API notifyRelease(const pthread_mutex_t* lock) noexcept;
    
    /** */
    static void QITI_API_INTERNAL resetAllListeners() noexcept;
    
private:
    LockData() = delete;
    ~LockData() = delete;
};
} // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
