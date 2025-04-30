
#include "qiti_utils.hpp"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cxxabi.h>
#include <dlfcn.h>
#include <iostream>
#include <map>
#include <string>

#ifndef QITI_DISABLE_HEAP_ALLOCATION_TRACKER
inline static thread_local uint64_t numHeapAllocationsOnCurrentThread = 0;
#endif

/** */
[[nodiscard]] static std::map<void*, qiti::FunctionData>& getFunctionMap()
{
    static std::map<void*, qiti::FunctionData> map;
    return map;
}

namespace qiti
{
struct FunctionData::FunctionCallData::Impl
{
    std::chrono::steady_clock::time_point begin_time;
    std::chrono::steady_clock::time_point end_time;
    int64_t timeSpentInFunctionNanoseconds = 0;
    
    int64_t numHeapAllocationsBeforeFunctionCall = 0;
    int64_t numHeapAllocationsAfterFunctionCall  = 0;
};

class FunctionData::Impl
{
public:
    QITI_API_INTERNAL Impl() = default;
    QITI_API_INTERNAL ~Impl() = default;
    
    std::string functionNameMangled;
    std::string functionNameReal;
    int64_t numTimesCalled = 0;
    int64_t averageTimeSpentInFunctionNanoseconds = 0;
    
    FunctionCallData lastCallData;
};

void printAllKnownFunctions()
{
    auto& g_functionMap = getFunctionMap();
    
    for (auto& [key, value] : g_functionMap)
    {
        std::cout << value.getFunctionName() << "\n";
    }
}

size_t getAllKnownFunctions(char* flatBuf,
                            size_t maxFunctions,
                            size_t maxNameLen)
{
    auto& map = getFunctionMap();
    size_t count = std::min(map.size(), maxFunctions);

    size_t i = 0;
    for (auto it = map.begin(); it != map.end() && i < count; ++it, ++i)
    {
        const std::string& name = it->second.getFunctionName();
        // copy up to maxNameLen-1 chars, then force '\0'
        std::strncpy(
          flatBuf + i * maxNameLen,
          name.c_str(),
          maxNameLen - 1
        );
        flatBuf[i * maxNameLen + (maxNameLen - 1)] = '\0';
    }

    return count;
}

void* getAddressForMangledFunctionName(const char* mangledName)
{
    void* addr = dlsym(RTLD_DEFAULT, mangledName);
    return addr;
}

[[nodiscard]] qiti::FunctionData& getFunctionData(void* functionAddress)
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

[[nodiscard]] const qiti::FunctionData* getFunctionData(const char* demangledFunctionName)
{
    auto& g_functionMap = getFunctionMap();
    
    auto it = std::find_if(g_functionMap.begin(), g_functionMap.end(),
                           [demangledFunctionName](const std::pair<void*, const qiti::FunctionData&>& pair)
                           {
                               return pair.second.getFunctionName() == std::string(demangledFunctionName);
                           });
    
    if (it == g_functionMap.end()) // function not found
        return nullptr;
    
    return &(it->second);
}
} // namespace qiti

extern "C" void QITI_API // Mark “no-instrument” to prevent recursing into itself
__cyg_profile_func_enter(void* this_fn, void* call_site)
{
    static int recursionCheck = 0;
    assert(++recursionCheck == 1);
    
    auto& functionData = qiti::getFunctionData(this_fn);
    auto* impl = functionData.getImpl();
    ++impl->numTimesCalled;
    impl->lastCallData.reset();
    impl->lastCallData.getImpl()->begin_time = std::chrono::steady_clock::now();
#ifndef QITI_DISABLE_HEAP_ALLOCATION_TRACKER
    impl->lastCallData.getImpl()->numHeapAllocationsBeforeFunctionCall = numHeapAllocationsOnCurrentThread;
#endif
    
    --recursionCheck;
}

extern "C" void QITI_API // Mark “no-instrument” to prevent recursing into itself
__cyg_profile_func_exit(void * this_fn, void* call_site)
{
    auto& functionData = qiti::getFunctionData(this_fn);
    auto* impl = functionData.getImpl();
    auto* callImpl = impl->lastCallData.getImpl();
    
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - callImpl->begin_time);

    callImpl->end_time = end_time;
    callImpl->timeSpentInFunctionNanoseconds = static_cast<int64_t>(elapsed_ns.count());
#ifndef QITI_DISABLE_HEAP_ALLOCATION_TRACKER
    callImpl->numHeapAllocationsAfterFunctionCall = numHeapAllocationsOnCurrentThread;
#endif
}

#ifndef QITI_DISABLE_HEAP_ALLOCATION_TRACKER
void* operator new(size_t size)
{
    ++numHeapAllocationsOnCurrentThread;
    void* p = malloc(size);
    return p;
}
#endif

namespace qiti
{

FunctionData::FunctionData(void* functionAddress)
{
    impl = new Impl;
    
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
    delete impl;
}

FunctionData::FunctionData(FunctionData&& other)
{
    impl = other.impl;
    other.impl = nullptr;
}

FunctionData& FunctionData::operator=(FunctionData&& other) noexcept
{
    assert(false);
}

FunctionData::Impl* FunctionData::getImpl() const noexcept
{
    return impl;
}

const char* FunctionData::getFunctionName() const noexcept
{
    return impl->functionNameReal.c_str();
}

qiti::uint FunctionData::getNumTimesCalled() const noexcept
{
    return impl->numTimesCalled;
}

const FunctionData::FunctionCallData* FunctionData::getLastFunctionCall() const noexcept
{
    return &impl->lastCallData;
}

FunctionData::FunctionCallData::FunctionCallData()
{
    impl = new Impl;
}

FunctionData::FunctionCallData::~FunctionCallData()
{
    delete impl;
}

FunctionData::FunctionCallData::FunctionCallData(FunctionData::FunctionCallData&& other)
{
    impl = other.impl;
    other.impl = nullptr;
}

FunctionData::FunctionCallData& FunctionData::FunctionCallData::operator=(FunctionData::FunctionCallData&& other) noexcept
{
    impl = other.impl;
    other.impl = nullptr;
}

FunctionData::FunctionCallData::Impl* FunctionData::FunctionCallData::getImpl() const noexcept
{
    return impl;
}

void FunctionData::FunctionCallData::reset() noexcept
{
    delete impl;
    impl = new Impl;
}

unsigned long long FunctionData::FunctionCallData::getNumHeapAllocations() const noexcept
{
    assert(impl->numHeapAllocationsAfterFunctionCall >= impl->numHeapAllocationsBeforeFunctionCall);
    return impl->numHeapAllocationsAfterFunctionCall - impl->numHeapAllocationsBeforeFunctionCall;
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
