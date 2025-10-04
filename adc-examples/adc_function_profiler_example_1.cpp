/******************************************************************************
 * Function entry/exit hooks that are called for every function
 * 
 * Clang flag required: -finstrument-functions
 ******************************************************************************/

extern "C"
{
    __attribute__((no_instrument_function))
    void __cyg_profile_func_enter(void* this_fn, void* call_site)
    {
        // Called when entering any function
    }

    __attribute__((no_instrument_function))
    void __cyg_profile_func_exit(void* this_fn, void* call_site)
    {
        // Called when exiting any function
    }
}
