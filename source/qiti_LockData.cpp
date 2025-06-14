
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_LockData.cpp
 *
 * @author   Adam Shield
 * @date     2025-05-25
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "qiti_LockData.hpp"

#include "qiti_LockHooks.hpp"

#include <mutex>
#include <ranges> // NOLINT - false positive in cpplint
#include <vector>

//--------------------------------------------------------------------------

using MutexType = std::mutex;
using LockType = std::scoped_lock<MutexType>;

inline static MutexType g_listenersMutex;
inline static std::vector<qiti::LockData::Listener*> g_listeners;

//--------------------------------------------------------------------------
namespace qiti
{
//--------------------------------------------------------------------------

void LockData::addGlobalListener(LockData::Listener* listener) noexcept
{
    qiti::LockHooks::LockBypassingHook<LockType, MutexType> lock(g_listenersMutex);
    g_listeners.push_back(listener);
}

void LockData::removeGlobalListener(LockData::Listener* listener) noexcept
{
    qiti::LockHooks::LockBypassingHook<LockType, MutexType> lock(g_listenersMutex);
    auto it = std::ranges::find(g_listeners, listener);
    if (it != g_listeners.end())
        g_listeners.erase(it);
}

void LockData::notifyAcquire(const pthread_mutex_t* lockAcquired) noexcept
{
    qiti::LockHooks::LockBypassingHook<LockType, MutexType> lock(g_listenersMutex);
    for (auto* l : g_listeners)
        l->onAcquire(lockAcquired);
}

void LockData::notifyRelease(const pthread_mutex_t* lockReleased) noexcept
{
    qiti::LockHooks::LockBypassingHook<LockType, MutexType> lock(g_listenersMutex);
    for (auto* l : g_listeners)
        l->onRelease(lockReleased);
}
//--------------------------------------------------------------------------
} // namespace qiti
//--------------------------------------------------------------------------
