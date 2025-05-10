
#include "qiti_ThreadSanitizer.hpp"

namespace qiti
{
ThreadSanitizer::ThreadSanitizer() noexcept
{
    
}

ThreadSanitizer::~ThreadSanitizer() noexcept
{
    
}

bool ThreadSanitizer::passedTest() noexcept
{
    return ! failed;  // TODO: implement
}

ThreadSanitizer ThreadSanitizer::functionsNotCalledInParallel(void* /*func0*/, void* /*func1*/)
{
    return {}; // TODO: implement
}

} // namespace qiti
