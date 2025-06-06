
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
#include "qiti_Profile.hpp"

#include <memory>
#include <mutex>

//--------------------------------------------------------------------------

std::mutex qiti_lock;

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
            
            std::scoped_lock<std::mutex> lock(qiti_lock);
            qiti::Profile::updateFunctionDataOnEnter(this_fn);
        }
    }
    
    static void QITI_API_INTERNAL
    __cyg_profile_func_exit(void * this_fn, [[maybe_unused]] void* call_site) noexcept
    {
        if (qiti::Profile::isProfilingFunction(this_fn))
        {
            qiti::MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;
            
            std::scoped_lock<std::mutex> lock(qiti_lock);
            qiti::Profile::updateFunctionDataOnExit(this_fn);
        }
    }
private:
    InstrumentHooks() = delete;
    ~InstrumentHooks() = delete;
};
} // namespace qiti

/** Hook exposed by -finstrument-functions called whenever entering instrumented function. */
extern "C" void QITI_API // Mark “no-instrument” to prevent recursing into itself
__cyg_profile_func_enter(void* this_fn, [[maybe_unused]] void* call_site) noexcept
{
    qiti::InstrumentHooks::__cyg_profile_func_enter(this_fn, call_site);
}

/** Hook exposed by -finstrument-functions called whenever exiting instrumented function. */
extern "C" void QITI_API // Mark “no-instrument” to prevent recursing into itself
__cyg_profile_func_exit(void * this_fn, [[maybe_unused]] void* call_site) noexcept
{
    qiti::InstrumentHooks::__cyg_profile_func_exit(this_fn, call_site);
}

//--------------------------------------------------------------------------

#if ! defined(__APPLE__)
// Linux-only:
// Force‐instantiate the char allocator, to prevent potential linker errors
// with its some of its function symbols not being resolved.
template class __attribute__((visibility("default"))) std::allocator<char>;
#endif // ! defined(__APPLE__)
