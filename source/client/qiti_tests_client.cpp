
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

// Qiti Shared API
#include "../../source/qiti_MallocHooks.hpp"

// TSAN
#include <sanitizer/common_interface_defs.h>

#include <stdlib.h>

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>

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
const char* __tsan_default_options()
{
    return TSAN_DEFAULT_OPTS;
}

#undef QITI_TSAN_LOG_PATH

//--------------------------------------------------------------------------

#ifdef QITI_ENABLE_THREAD_SANITIZER
#if ! defined(__APPLE__)
// When ThreadSanitizer is enabled, Linux uses __sanitizer_malloc_hook
__attribute__((no_sanitize_thread))
extern "C" void __sanitizer_malloc_hook(void* /*ptr*/,
                                        size_t size)
{
    qiti::MallocHooks::mallocHook(size);
}
#endif // ! defined(__APPLE__)
// When ThreadSanitizer is disabled, Linux will use operator new override instead (matching macOS implementation)
#endif // QITI_ENABLE_THREAD_SANITIZER

//--------------------------------------------------------------------------
