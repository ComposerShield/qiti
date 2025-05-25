
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_client.hpp
 *
 * @author   Adam Shield
 * @date     2025-05-16
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#define QITI_TSAN_LOG_PATH "/tmp/tsan.log"

static constexpr const char TSAN_DEFAULT_OPTS[] =
    "report_thread_leaks=0:abort_on_error=0:log_path=" QITI_TSAN_LOG_PATH;

extern "C"
__attribute__((visibility("default")))
__attribute__((no_sanitize("thread")))
__attribute__((noinline))
const char* __tsan_default_options()
{
    return TSAN_DEFAULT_OPTS;
}

#undef QITI_TSAN_LOG_PATH
