
#include "qiti_utils.hpp"

#include <cassert>
#include <cstdio>
#include <cxxabi.h>
#include <dlfcn.h>
#include <map>

#include <iostream>

/** */
[[nodiscard]] static std::map<void*, qiti::FunctionData>& getFunctionMap()
{
    static std::map<void*, qiti::FunctionData> map;
    return map;
}

/** */
extern "C" [[nodiscard]] qiti::FunctionData& __attribute__((no_instrument_function))
getFunctionData(void* functionAddress)
{
    auto& g_functionMap = getFunctionMap();
    
    auto it = g_functionMap.find(functionAddress);
    if (it == g_functionMap.end()) // not yet added to map
    {
        Dl_info info;
        if (dladdr(functionAddress, &info))
        {
            qiti::FunctionData data;
//            data.functionNameMangled = info.dli_sname;
//            data.functionNameReal = qiti::demangle(info.dli_sname);
            
            auto [insertedIt, success] = g_functionMap.emplace(functionAddress, std::move(data));
            return insertedIt->second;
        }
        else
        {
            std::terminate();
            // Handle the rare case where dladdr fails: still must insert something.
            auto [insertedIt, success] = g_functionMap.emplace(functionAddress, qiti::FunctionData{});
            return insertedIt->second;
        }
    }
    
    return it->second;
}

// Mark “no-instrument” to prevent recursing into itself
extern "C" void QITI_API
__cyg_profile_func_enter(void* this_fn, void* call_site)
{
    static int recursionCheck = 0;
    assert(++recursionCheck == 1);
    
    [[maybe_unused]] auto& functionData = getFunctionData(this_fn);
//    ++functionData.numTimesCalled;
//    functionData.lastCallData.begin_time = std::chrono::steady_clock::now();
    
    --recursionCheck;
}

// Mark “no-instrument” to prevent recursing into itself
extern "C" void QITI_API
__cyg_profile_func_exit(void * this_fn, void* call_site)
{
//    auto& functionData = getFunctionData(this_fn);
//    auto end_time = std::chrono::steady_clock::now();
//    auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - functionData.lastCallData.begin_time);
//
//    functionData.lastCallData.end_time = end_time;
//    functionData.lastCallData.timeSpentInFunction = static_cast<qiti::time_ns>(elapsed_ns.count());
}

namespace qiti
{
//std::string demangle(const char* mangledName)
//{
//    int status = 0;
//    // abi::__cxa_demangle allocates with malloc; we wrap it in unique_ptr so it frees automatically
//    std::unique_ptr<char, void(*)(void*)> result
//    {
//        abi::__cxa_demangle(mangledName, nullptr, nullptr, &status),
//        std::free
//    };
//    // on success, result.get() is our demangled C-string; otherwise fall back
//    return (status == 0 && result) ? result.get() : mangledName;
//}

void demangle(const char* mangled_name, char* demangled_name, unsigned long long demangled_size)
{
    int status = 0;
    char* result = abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);

    if (status == 0 && result != nullptr)
    {
        // Safely copy into caller's buffer
        std::strncpy(demangled_name, result, demangled_size - 1);
        demangled_name[demangled_size - 1] = '\0'; // always null-terminate
        std::free(result);
    }
    else
    {
        // fallback: copy mangled name itself
        std::strncpy(demangled_name, mangled_name, demangled_size - 1);
        demangled_name[demangled_size - 1] = '\0'; // always null-terminate
    }
}
} // namespace qiti
