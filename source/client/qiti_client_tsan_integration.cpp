
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_client_tsan_integration.cpp
 *
 * @author   Adam Shield
 * @date     2025-05-25
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

// Qiti Shared API
#include "../../source/qiti_MallocHooks.hpp"
#include "../../source/qiti_LockHooks.hpp"

// TSAN
#include <sanitizer/common_interface_defs.h>
#include <sanitizer/tsan_interface.h>

#include <stdlib.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <unordered_map>


//--------------------------------------------------------------------------

#define QITI_TSAN_LOG_PATH "/tmp/tsan.log"

static constexpr const char TSAN_DEFAULT_OPTS[] = "report_thread_leaks=0"
                                                  ":abort_on_error=0"
                                                  ":log_path="
                                                  QITI_TSAN_LOG_PATH;

extern "C"
__attribute__((visibility("default")))
__attribute__((no_sanitize("thread")))
__attribute__((noinline))
#if ! defined(_WIN32)
QITI_API
#endif
const char* __tsan_default_options()
{
    return TSAN_DEFAULT_OPTS;
}

#undef QITI_TSAN_LOG_PATH

//--------------------------------------------------------------------------

#ifdef QITI_ENABLE_THREAD_SANITIZER
#if ! defined(__APPLE__)
// Linux with ThreadSanitizer uses TSan hooks for allocation tracking

// Recursion guard for Linux TSan hooks
static thread_local bool g_insideTSanHook = false;

// When ThreadSanitizer is enabled, Linux uses __sanitizer_malloc_hook instead of
// operator new because that operator is already used by TSan
__attribute__((no_sanitize_thread))
extern "C" QITI_API void __sanitizer_malloc_hook([[maybe_unused]] void* ptr, size_t size)
{
    if (g_insideTSanHook)
        return;
    g_insideTSanHook = true;
    
    qiti::MallocHooks::mallocHookWithTracking(ptr, size);
    
    g_insideTSanHook = false;
}

// When ThreadSanitizer is enabled, Linux uses __sanitizer_free_hook instead of
// operator delete because that operator is already used by TSan
__attribute__((no_sanitize_thread))
extern "C" QITI_API void __sanitizer_free_hook(void* ptr)
{
    if (g_insideTSanHook)
        return;
    g_insideTSanHook = true;
    
    qiti::MallocHooks::freeHookWithTracking(ptr);
    
    g_insideTSanHook = false;
}


#endif // ! defined(__APPLE__)
// When ThreadSanitizer is disabled, Linux will use operator new override instead (matching macOS implementation)
#endif // QITI_ENABLE_THREAD_SANITIZER

//--------------------------------------------------------------------------

#ifdef _WIN32
// Windows operator new/delete overrides - must be in executable, not DLL
// to properly override standard library operators

void* operator new(std::size_t size)
{
    void* ptr = std::malloc(size);
    if (ptr == nullptr)
        throw std::bad_alloc{};
    
    if (! qiti::MallocHooks::bypassMallocHooks)
        qiti::MallocHooks::mallocHookWithTracking(ptr, size);
    
    return ptr;
}

void* operator new[](std::size_t size)
{
    void* ptr = std::malloc(size);
    if (ptr == nullptr)
        throw std::bad_alloc{};
    
    if (! qiti::MallocHooks::bypassMallocHooks)
        qiti::MallocHooks::mallocHookWithTracking(ptr, size);
    
    return ptr;
}

void operator delete(void* ptr) noexcept
{
    if (ptr != nullptr)
    {
        if (! qiti::MallocHooks::bypassMallocHooks)
            qiti::MallocHooks::freeHookWithTracking(ptr);
        std::free(ptr);
    }
}

void operator delete[](void* ptr) noexcept
{
    if (ptr != nullptr)
    {
        if (! qiti::MallocHooks::bypassMallocHooks)
            qiti::MallocHooks::freeHookWithTracking(ptr);
        std::free(ptr);
    }
}

// Sized delete operators (C++14)
void operator delete(void* ptr, std::size_t /*size*/) noexcept
{
    if (ptr != nullptr)
    {
        if (! qiti::MallocHooks::bypassMallocHooks)
            qiti::MallocHooks::freeHookWithTracking(ptr);
        std::free(ptr);
    }
}

void operator delete[](void* ptr, std::size_t /*size*/) noexcept
{
    if (ptr != nullptr)
    {
        if (! qiti::MallocHooks::bypassMallocHooks)
            qiti::MallocHooks::freeHookWithTracking(ptr);
        std::free(ptr);
    }
}

#endif // _WIN32

//--------------------------------------------------------------------------
