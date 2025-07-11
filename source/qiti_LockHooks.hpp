
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

#include <pthread.h>

#include <memory>

//--------------------------------------------------------------------------
// Doxygen - Begin Internal Documentation
/** \cond INTERNAL */
//--------------------------------------------------------------------------

namespace qiti
{
//--------------------------------------------------------------------------
/**
 */
class LockHooks
{
private:
    struct DummyMutex {};
    struct DummyLock  { DummyLock(const DummyMutex&) {} };
    
public:
    inline static thread_local bool bypassLockHooks = false;

    template<typename _LockType, typename _MutexType>
    struct LockBypassingHook
    {
        using LockType  = _LockType;
        using MutexType = _MutexType;
        
        /** Temporarily disable malloc hooks for the current thread for however long this object is in scope. */
        inline QITI_API_INTERNAL LockBypassingHook(MutexType& mutex) noexcept
        : previousBypassState(bypassLockHooks)
        {
            bypassLockHooks = true;
            lock = std::make_unique<LockType>(mutex);
        }
        
        /** On destruction, resets bypassMallocHooks to the value saved at construction. */
        inline QITI_API_INTERNAL ~LockBypassingHook() noexcept
        {
            lock.reset();
            bypassLockHooks = previousBypassState;
        }
        
    private:
        const bool previousBypassState;
        std::unique_ptr<LockType> lock;
    };
    
    struct ScopedDisableHooks
    : public LockBypassingHook<DummyLock, DummyMutex>
    {
        ScopedDisableHooks() : LockBypassingHook(mutex) {}
        
    private:
        DummyMutex mutex;
    };
    
    /** */
    static void QITI_API lockAcquireHook(const pthread_mutex_t* size) noexcept;
    /** */
    static void QITI_API lockReleaseHook(const pthread_mutex_t* size) noexcept;

private:
    LockHooks() = delete;
    ~LockHooks() = delete;
};
} // namespace qiti

//--------------------------------------------------------------------------
/** \endcond */
// Doxygen - End Internal Documentation
//--------------------------------------------------------------------------
