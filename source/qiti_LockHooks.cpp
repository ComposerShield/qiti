
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_LockHooks.cpp
 *
 * @author   Adam Shield
 * @date     2025-06-13
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "qiti_LockData.hpp"

#include "qiti_LockHooks.hpp"

#include <pthread.h>

#include <cstdio>

//--------------------------------------------------------------------------

extern bool isQitiTestRunning() noexcept;

[[maybe_unused]] /** TODO: remove when supporting Linux. */
static thread_local bool g_inHook = false;

//--------------------------------------------------------------------------

#if defined(__APPLE__) // only supported on MacOS currently
extern "C" int my_pthread_mutex_lock(pthread_mutex_t* m) noexcept
{
    if (isQitiTestRunning())
    {
        if (! qiti::LockHooks::bypassLockHooks)
        {
            if (! g_inHook)
            {
                g_inHook = true;
                qiti::LockHooks::lockAcquireHook(m);
                g_inHook = false;
            }
        }
    }
    return pthread_mutex_lock(m);
}

extern "C" int my_pthread_mutex_unlock(pthread_mutex_t* m) noexcept
{
    if (isQitiTestRunning())
    {
        if (! qiti::LockHooks::bypassLockHooks)
        {
            if (! g_inHook)
            {
                g_inHook = true;
                qiti::LockHooks::lockReleaseHook(m);
                g_inHook = false;
            }
        }
    }
    return pthread_mutex_unlock(m);
}

// The interpose array must live in its own section:
__attribute__((used))
static struct
{
    const void* replacement;
    const void* original;
}
interposers[]
__attribute__((section("__DATA,__interpose"))) =
{
    { reinterpret_cast<const void*>(my_pthread_mutex_lock),   reinterpret_cast<const void*>(pthread_mutex_lock)   },
    { reinterpret_cast<const void*>(my_pthread_mutex_unlock), reinterpret_cast<const void*>(pthread_mutex_unlock) },
};
#endif // defined(__APPLE__)

namespace qiti
{
void LockHooks::lockAcquireHook(const pthread_mutex_t* mutex) noexcept
{
    qiti::LockData::notifyAcquire(mutex);
}

void LockHooks::lockReleaseHook(const pthread_mutex_t* mutex) noexcept
{
    qiti::LockData::notifyRelease(mutex);
}
} // namespace qiti
