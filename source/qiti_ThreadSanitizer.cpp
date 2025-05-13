
#include "qiti_ThreadSanitizer.hpp"

const char* __tsan_default_options()
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
