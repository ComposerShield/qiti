
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_InstrumentHooks.cpp
 *
 * @author   Adam Shield
 * @date     2025-06-06
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "qiti_MallocHooks.hpp"

#include "qiti_LockHooks.hpp"
#include "qiti_Profile.hpp"

#include <memory>
#include <mutex>

//--------------------------------------------------------------------------

using MutexType = std::mutex;
using LockType = std::scoped_lock<MutexType>;

static MutexType g_hookLock;
static thread_local bool g_inHook = false;

//--------------------------------------------------------------------------
namespace qiti
{
//--------------------------------------------------------------------------
class InstrumentHooks
{
public:
    static void QITI_API_INTERNAL
    __cyg_profile_func_enter(void* this_fn, [[maybe_unused]] void* call_site) noexcept
    {
        if (qiti::Profile::isProfilingFunction(this_fn))
        {
            qiti::MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;
            
            qiti::LockHooks::LockBypassingHook<LockType, MutexType> lock(g_hookLock);
            qiti::Profile::updateFunctionDataOnEnter(this_fn);
        }
    }
    
    static void QITI_API_INTERNAL
    __cyg_profile_func_exit(void * this_fn, [[maybe_unused]] void* call_site) noexcept
    {
        if (qiti::Profile::isProfilingFunction(this_fn))
        {
            qiti::MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;
            
            qiti::LockHooks::LockBypassingHook<LockType, MutexType> lock(g_hookLock);
            qiti::Profile::updateFunctionDataOnExit(this_fn);
        }
    }
private:
    InstrumentHooks() = delete;
    ~InstrumentHooks() = delete;
};
} // namespace qiti

//--------------------------------------------------------------------------

/** Hook exposed by -finstrument-functions called whenever entering instrumented function. */
extern "C" void QITI_API // Mark “no-instrument” to prevent recursing into itself
__cyg_profile_func_enter(void* this_fn, [[maybe_unused]] void* call_site) noexcept
{
    if (g_inHook)
        return;       // already in our hook, bail out
    g_inHook = true;  // mark “in hook” for this thread
    
    qiti::InstrumentHooks::__cyg_profile_func_enter(this_fn, call_site);
    
    g_inHook = false; // un-mark
}

/** Hook exposed by -finstrument-functions called whenever exiting instrumented function. */
extern "C" void QITI_API // Mark “no-instrument” to prevent recursing into itself
__cyg_profile_func_exit(void * this_fn, [[maybe_unused]] void* call_site) noexcept
{
    if (g_inHook)
        return;
    g_inHook = true;
    
    qiti::InstrumentHooks::__cyg_profile_func_exit(this_fn, call_site);
    
    g_inHook = false;
}

//--------------------------------------------------------------------------

#if ! defined(__APPLE__)
// Linux-only:
// Force‐instantiate the char allocator, to prevent potential linker errors
// with its some of its function symbols not being resolved.
template class __attribute__((visibility("default"))) std::allocator<char>;
#endif // ! defined(__APPLE__)
