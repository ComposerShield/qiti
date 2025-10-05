/******************************************************************************
 * Track how many times each function is called
 * 
 * Clang flag required: -finstrument-functions
 ******************************************************************************/

#include <unordered_map>
#include <cstddef>

extern "C"
{
    __attribute__((no_instrument_function))
    void __cyg_profile_func_enter(void* this_fn, void* call_site)
    {
        if (! shouldTrackCall)
            return;
        
        // ...

        // You could also query:
        // - average/min/max time spent in each function
        // - time spent in function the last time it was called
        // - which thread is calling this function
        // - how many memory allocations and how much memory allocated
        // - which function called this function
        // - which functions are called the most often (hotpath)
        // - etc.
    }

    __attribute__((no_instrument_function))
    void __cyg_profile_func_exit(void* this_fn, void* call_site)
    {

    }
}
