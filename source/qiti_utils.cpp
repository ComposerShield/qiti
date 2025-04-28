
#include "qiti_utils.hpp"

#include <cassert>
#include <cstdio>
#include <cxxabi.h>
#include <dlfcn.h>
#include <map>
#include <string>

/** */
[[nodiscard]] static std::map<void*, qiti::FunctionData>& getFunctionMap()
{
    static std::map<void*, qiti::FunctionData> map;
    return map;
}

namespace qiti
{
class FunctionData::FunctionDataImpl
{
public:
    QITI_API_INTERNAL FunctionDataImpl() = default;
    QITI_API_INTERNAL ~FunctionDataImpl() = default;
    
    std::string functionNameMangled;
    std::string functionNameReal;
    int64_t numTimesCalled = 0;
    int64_t averageTimeSpentInFunctionNanoseconds = 0;

    struct LastCallData
    {
        std::chrono::steady_clock::time_point begin_time;
        std::chrono::steady_clock::time_point end_time;
        int64_t timeSpentInFunctionNanoseconds = 0;

        int32_t numHeapAllocations = 0;
    };
    LastCallData lastCallData;
};
} // namespace qiti

/** */
extern "C" [[nodiscard]] qiti::FunctionData& __attribute__((no_instrument_function))
getFunctionData(void* functionAddress)
{
    auto& g_functionMap = getFunctionMap();
    
    auto it = g_functionMap.find(functionAddress);
    if (it == g_functionMap.end()) // not yet added to map
    {
        qiti::FunctionData data(functionAddress);
        
        auto [insertedIt, success] = g_functionMap.emplace(functionAddress, std::move(data));
        return insertedIt->second;
    }
    
    return it->second;
}

[[nodiscard]] const qiti::FunctionData* QITI_API_INTERNAL getFunctionData(const char* demangledFunctionName)
{
    auto& g_functionMap = getFunctionMap();
    
    auto it = std::find_if(g_functionMap.begin(), g_functionMap.end(),
        [demangledFunctionName](const std::pair<void*, qiti::FunctionData>& pair)
        {
            return pair.second.getFunctionName() == std::string(demangledFunctionName);
        });
    
    if (it == g_functionMap.end()) // function not found
        return nullptr;
    
    return &(it->second);
}

// Mark “no-instrument” to prevent recursing into itself
extern "C" void QITI_API
__cyg_profile_func_enter(void* this_fn, void* call_site)
{
    static int recursionCheck = 0;
    assert(++recursionCheck == 1);
    
    auto& functionData = getFunctionData(this_fn);
    auto* impl = functionData.getImpl();
    ++impl->numTimesCalled;
    impl->lastCallData.begin_time = std::chrono::steady_clock::now();
    
    --recursionCheck;
}

// Mark “no-instrument” to prevent recursing into itself
extern "C" void QITI_API
__cyg_profile_func_exit(void * this_fn, void* call_site)
{
    auto& functionData = getFunctionData(this_fn);
    auto* impl = functionData.getImpl();
    
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - impl->lastCallData.begin_time);

    impl->lastCallData.end_time = end_time;
    impl->lastCallData.timeSpentInFunctionNanoseconds = static_cast<int64_t>(elapsed_ns.count());
}

namespace qiti
{

FunctionData::FunctionData(void* functionAddress)
{
    impl = new FunctionDataImpl;
    
    Dl_info info;
    if (dladdr(functionAddress, &info))
    {
        impl->functionNameMangled = info.dli_sname;
        char functionNameDemangled[256];
        qiti::demangle(info.dli_sname, functionNameDemangled, sizeof(functionNameDemangled));
        impl->functionNameReal = functionNameDemangled;
    }
    else
    {
        std::terminate(); // TODO: will this ever happen?
    }
}

FunctionData::~FunctionData()
{
//    delete impl; // TODO: Causes crash...
}

FunctionData::FunctionDataImpl* FunctionData::getImpl() const
{
    return impl;
}

const char* FunctionData::getFunctionName() const noexcept
{
    return impl->functionNameReal.c_str();
}

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

void shutdown()
{
    getFunctionMap().clear();
}
} // namespace qiti
