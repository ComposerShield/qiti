
#include <qiti_utils.hpp>

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cxxabi.h>
#include <dlfcn.h>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <unordered_set>

#include "qiti_include.hpp"

#include "qiti_FunctionCallData_Impl.hpp"
#include "qiti_FunctionCallData.hpp"
#include "qiti_FunctionData_Impl.hpp"
#include "qiti_FunctionData.hpp"

//--------------------------------------------------------------------------

#ifndef QITI_DISABLE_HEAP_ALLOCATION_TRACKER
inline static thread_local uint64_t g_numHeapAllocationsOnCurrentThread = 0;
extern thread_local std::function<void()> g_onNextHeapAllocation;
#endif

/** */
std::recursive_mutex qiti_global_lock;

/** */
[[nodiscard]] static auto& getFunctionMap() noexcept
{
    static std::unordered_map<void*, qiti::FunctionData> map;
    return map;
}

namespace qiti
{
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

[[nodiscard]] qiti::FunctionData& getFunctionDataFromAddress(void* functionAddress)
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

void demangle(const char* mangled_name, char* demangled_name, uint demangled_size)
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

void resetAll()
{
    // Prevent any qiti work while we reset everything
    std::scoped_lock<std::recursive_mutex> lock(qiti_global_lock);
    
    getFunctionMap().clear();
    
    instrument::resetInstrumentation();
    profile::resetProfiling();
}
} // namespace qiti

static void QITI_API_INTERNAL updateFunctionDataOnEnter(void* this_fn) noexcept
{
    auto& functionData = qiti::getFunctionDataFromAddress(this_fn);
    auto* impl = functionData.getImpl();
    ++impl->numTimesCalled;
    impl->lastCallData.reset(); // Deletes previous impl
    
    auto* lastCallImpl = impl->lastCallData.getImpl();
    lastCallImpl->begin_time = std::chrono::steady_clock::now();
    lastCallImpl->callingThread = std::this_thread::get_id();
#ifndef QITI_DISABLE_HEAP_ALLOCATION_TRACKER
    impl->lastCallData.getImpl()->numHeapAllocationsBeforeFunctionCall = g_numHeapAllocationsOnCurrentThread;
#endif
}

static void QITI_API_INTERNAL updateFunctionDataOnExit(void* this_fn) noexcept
{
    auto& functionData = qiti::getFunctionDataFromAddress(this_fn);
    auto* impl = functionData.getImpl();
    auto* callImpl = impl->lastCallData.getImpl();
    
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - callImpl->begin_time);

    callImpl->end_time = end_time;
    callImpl->timeSpentInFunctionNanoseconds = static_cast<qiti::uint>(elapsed_ns.count());
#ifndef QITI_DISABLE_HEAP_ALLOCATION_TRACKER
    callImpl->numHeapAllocationsAfterFunctionCall = g_numHeapAllocationsOnCurrentThread;
#endif
}

extern "C" void QITI_API // Mark “no-instrument” to prevent recursing into itself
__cyg_profile_func_enter(void* this_fn, [[maybe_unused]] void* call_site)
{
    static int recursionCheck = 0;
    assert(++recursionCheck == 1);
    
    qiti_global_lock.lock(); // lock until end of function call in __cyg_profile_func_exit
    
    if (qiti::profile::shouldProfileFunction(this_fn))
        updateFunctionDataOnEnter(this_fn);
    
    --recursionCheck;
}

extern "C" void QITI_API // Mark “no-instrument” to prevent recursing into itself
__cyg_profile_func_exit(void * this_fn, [[maybe_unused]] void* call_site)
{
    if (qiti::profile::shouldProfileFunction(this_fn))
        updateFunctionDataOnExit(this_fn);
    
    qiti_global_lock.unlock(); // lock was held since __cyg_profile_func_enter
}

#ifndef QITI_DISABLE_HEAP_ALLOCATION_TRACKER
void* operator new(size_t size)
{
    ++g_numHeapAllocationsOnCurrentThread;
    if (auto callback = std::exchange(g_onNextHeapAllocation, nullptr))
        callback();
    
    // Original implementation
    void* p = malloc(size);
    return p;
}
#endif
