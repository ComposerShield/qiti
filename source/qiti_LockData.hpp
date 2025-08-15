
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
 Lock instrumentation data management and listener notification system.

 The LockData class provides mechanisms for tracking mutex lock acquisitions
 and releases across the application. It maintains a registry of listeners
 that can be notified when locks are acquired or released, enabling
 comprehensive lock profiling and deadlock detection.

 @note This class is designed for internal use by the Qiti profiling system.
 */
class LockData
{
public:
    struct Listener
    {
        QITI_API Listener() noexcept = default;
        QITI_API virtual ~Listener() noexcept = default;
        /** User provided callback. */
        QITI_API virtual void onAcquire(const pthread_mutex_t* ld) noexcept = 0;
        /** User provided callback. */
        QITI_API virtual void onRelease(const pthread_mutex_t* ld) noexcept = 0;
    };
    
    /** Register for lock/unlock notifications. */
    QITI_API static void addGlobalListener(Listener* listener) noexcept;
    /** Unregister for lock/unlock notifications. */
    QITI_API static void removeGlobalListener(Listener* listener) noexcept;
    
    /** Notify listeners of a lock acquisition */
    QITI_API static void notifyAcquire(const pthread_mutex_t* lock) noexcept;
    /** Notify listeners of a lock release */
    QITI_API static void notifyRelease(const pthread_mutex_t* lock) noexcept;
    
    /** */
    QITI_API_INTERNAL static void resetAllListeners() noexcept;
    
private:
    LockData() = delete;
    ~LockData() = delete;
};
} // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
