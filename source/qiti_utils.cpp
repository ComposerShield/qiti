
#include "qiti_utils.hpp"

#include <dlfcn.h>
#include <cstdio>
#include <cxxabi.h>

namespace qiti
{

std::string demangle(const char* mangledName)
{
    int status = 0;
    // abi::__cxa_demangle allocates with malloc; we wrap it in unique_ptr so it frees automatically
    std::unique_ptr<char, void(*)(void*)> result
    {
        abi::__cxa_demangle(mangledName, nullptr, nullptr, &status),
        std::free
    };
    // on success, result.get() is our demangled C-string; otherwise fall back
    return (status == 0 && result) ? result.get() : mangledName;
}

// Mark “no-instrument” to prevent recursing into itself
extern "C" void __attribute__((no_instrument_function))
__cyg_profile_func_enter(void *this_fn, void *call_site)
{
    Dl_info info;
    if (dladdr(this_fn, &info) && info.dli_sname)
    {
        // e.g. record timestamp for info.dli_sname
        printf("ENTER %s\n", info.dli_sname);
    }
}

// Mark “no-instrument” to prevent recursing into itself
extern "C" void __attribute__((no_instrument_function))
__cyg_profile_func_exit(void *this_fn, void *call_site)
{
    Dl_info info;
    if (dladdr(this_fn, &info) && info.dli_sname)
    {
        // record exit timestamp
        printf(" EXIT %s\n", info.dli_sname);
    }
}

} // namespace qiti
