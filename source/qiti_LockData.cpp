
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

#include <mutex>
#include <vector>

//--------------------------------------------------------------------------

std::mutex s_listenersMutex;
std::vector<qiti::LockData::Listener*> s_listeners;

//--------------------------------------------------------------------------
namespace qiti
{
//--------------------------------------------------------------------------
void LockData::addGlobalListener(LockData::Listener* listener) noexcept
{
    std::scoped_lock<std::mutex> guard(s_listenersMutex);
    s_listeners.push_back(listener);
}

void LockData::removeGlobalListener(LockData::Listener* listener) noexcept
{
    std::scoped_lock<std::mutex> guard(s_listenersMutex);
    auto it = std::find(s_listeners.begin(), s_listeners.end(), listener);
    if (it != s_listeners.end())
        s_listeners.erase(it);
}

void LockData::notifyAcquire() noexcept
{
    std::scoped_lock<std::mutex> guard(s_listenersMutex);
    for (auto* l : s_listeners)
        l->onAcquire(this);
}

void LockData::notifyRelease() noexcept
{
    std::scoped_lock<std::mutex> guard(s_listenersMutex);
    for (auto* l : s_listeners)
        l->onRelease(this);
}
//--------------------------------------------------------------------------
} // namespace qiti
//--------------------------------------------------------------------------
