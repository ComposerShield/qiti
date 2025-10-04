/******************************************************************************
 * Add recursion blocker to prevent infinite recursion
 * 
 * Clang flag required: -finstrument-functions
 ******************************************************************************/

static thread_local bool shouldTrackCall = true;

extern "C"
{
    __attribute__((no_instrument_function))
    void __cyg_profile_func_enter(void* this_fn, void* call_site)
    {
        if (! shouldTrackCall)
            return;
        
        shouldTrackCall = false;
        // do stuff...
        shouldTrackCall = true;
    }

    __attribute__((no_instrument_function))
    void __cyg_profile_func_exit(void* this_fn, void* call_site)
    {

    }
}
