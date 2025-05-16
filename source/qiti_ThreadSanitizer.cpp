
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_ThreadSanitizer.cpp
 *
 * @author   Adam Shield
 * @date     2025-05-16
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "qiti_ThreadSanitizer.hpp"

//--------------------------------------------------------------------------

#define QITI_TSAN_LOG_PATH "/tmp/tsan.log"
static constexpr const char TSAN_DEFAULT_OPTS[] =
    "report_thread_leaks=0:abort_on_error=0:log_path=" QITI_TSAN_LOG_PATH;

extern "C" const char* __tsan_default_options()
{
    return TSAN_DEFAULT_OPTS;
}

//--------------------------------------------------------------------------

namespace qiti
{
ThreadSanitizer::ThreadSanitizer() noexcept
{
    
}

ThreadSanitizer::~ThreadSanitizer() noexcept
{
    
}

bool ThreadSanitizer::passed() noexcept
{
    return ! _failed;  // TODO: implement
}

ThreadSanitizer ThreadSanitizer::functionsNotCalledInParallel(void* /*func0*/, void* /*func1*/)
{
    return {}; // TODO: implement
}

} // namespace qiti
