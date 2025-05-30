
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_tests_client.cpp
 *
 * @author   Adam Shield
 * @date     2025-05-25
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

 #include <stdlib.h>

 #include <cstdint>
 #include <functional>
 #include <mutex>

 //--------------------------------------------------------------------------

#define QITI_TSAN_LOG_PATH "/tmp/tsan.log"

thread_local std::recursive_mutex bypassMallocHooksLock;
thread_local uint64_t g_numHeapAllocationsOnCurrentThread = 0;
thread_local std::function<void()> g_onNextHeapAllocation = nullptr;

static constexpr const char TSAN_DEFAULT_OPTS[] = "report_thread_leaks=0"
                                                  ":abort_on_error=0"
                                                  ":log_path="
                                                  QITI_TSAN_LOG_PATH;

//--------------------------------------------------------------------------

extern "C"
__attribute__((visibility("default")))
__attribute__((no_sanitize("thread")))
__attribute__((noinline))
const char* __tsan_default_options()
{
    return TSAN_DEFAULT_OPTS;
}

#undef QITI_TSAN_LOG_PATH

//--------------------------------------------------------------------------

extern "C" void __sanitizer_malloc_hook([[maybe_unused]] void* ptr, 
                                        [[maybe_unused]] size_t size) 
{
    std::unique_lock lock(bypassMallocHooksLock, std::try_to_lock);
    
    if (lock.owns_lock())
    {
        ++g_numHeapAllocationsOnCurrentThread;
        if (g_onNextHeapAllocation != nullptr)
        {
            g_onNextHeapAllocation();
            g_onNextHeapAllocation = nullptr;
        }
    }
}

extern "C" void __sanitizer_free_hook([[maybe_unused]] void* ptr) 
{
    // TODO: implement
}

//--------------------------------------------------------------------------