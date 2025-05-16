
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

const char* QITI_API __tsan_default_options()
{
    // no leak-on-exit, no abort on race, write report to tsan.log
    return "report_thread_leaks=0:abort_on_error=0:log_path=/tmp/tsan.log";
}


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
