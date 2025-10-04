/******************************************************************************
 * Track how many times each function is called
 * 
 * Clang flag required: -finstrument-functions
 ******************************************************************************/

#include <unordered_map>
#include <cstddef>

static std::unordered_map<void*, std::size_t> callCounts;
static thread_local bool shouldTrackCall = true;

__attribute__((no_instrument_function))
std::size_t getCallCount(void* func)
{
    auto it = callCounts.find(func);
    return (it != callCounts.end()) ? it->second : 0;
}

extern "C"
{
    __attribute__((no_instrument_function))
    void __cyg_profile_func_enter(void* this_fn, void* call_site)
    {
        if (! shouldTrackCall)
            return;
        
        shouldTrackCall = false;
        callCounts[this_fn]++;
        shouldTrackCall = true;
    }

    __attribute__((no_instrument_function))
    void __cyg_profile_func_exit(void* this_fn, void* call_site)
    {

    }
}
