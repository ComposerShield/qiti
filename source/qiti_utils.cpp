
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_utils.cpp
 *
 * @author   Adam Shield
 * @date     2025-05-16
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include <qiti_utils.hpp>

#include "qiti_include.hpp"
#include "qiti_instrument.hpp"

#include "qiti_ReentrantSharedMutex.hpp"

#include <cxxabi.h>
#include <dlfcn.h>

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <map>
#include <mutex>
//#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

//--------------------------------------------------------------------------

/** */
qiti::ReentrantSharedMutex qiti_global_lock;

std::mutex qiti_lock;

/** */
[[nodiscard]] static auto& getFunctionMap() noexcept
{
    static std::unordered_map<void*, qiti::FunctionData> map;
    return map;
}

namespace qiti
{
size_t getAllKnownFunctions(char* /*flatBuf*/,
                            size_t /*maxFunctions*/,
                            size_t /*maxNameLen*/) noexcept
{
    return 0;
//    auto& map = getFunctionMap();
//    size_t count = std::min(map.size(), maxFunctions);
//
//    size_t i = 0;
//    for (auto it = map.begin(); it != map.end() && i < count; ++it, ++i)
//    {
//        const std::string& name = it->second.getFunctionName();
//        // copy up to maxNameLen-1 chars, then force '\0'
//        std::strncpy(
//          flatBuf + i * maxNameLen,
//          name.c_str(),
//          maxNameLen - 1
//        );
//        flatBuf[i * maxNameLen + (maxNameLen - 1)] = '\0';
//    }
//
//    return count;
}

void* getAddressForMangledFunctionName(const char* mangledName) noexcept
{
    void* addr = dlsym(RTLD_DEFAULT, mangledName);
    return addr;
}

[[nodiscard]] qiti::FunctionData& getFunctionDataFromAddress(void* functionAddress) noexcept
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

[[nodiscard]] const qiti::FunctionData* getFunctionData(const char* /*demangledFunctionName*/) noexcept
{
    return nullptr;
//    auto& g_functionMap = getFunctionMap();
//    
//    auto it = std::find_if(g_functionMap.begin(), g_functionMap.end(),
//                           [demangledFunctionName](const std::pair<void*, const qiti::FunctionData&>& pair)
//                           {
//                               return pair.second.getFunctionName() == std::string(demangledFunctionName);
//                           });
//    
//    if (it == g_functionMap.end()) // function not found
//        return nullptr;
//    
//    return &(it->second);
}

void demangle(const char* /*mangled_name*/, char* /*demangled_name*/, uint64_t /*demangled_size*/) noexcept
{
//    int status = 0;
//    char* result = abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);
//
//    if (status == 0 && result != nullptr)
//    {
//        // Safely copy into caller's buffer
//        std::strncpy(demangled_name, result, demangled_size - 1);
//        demangled_name[demangled_size - 1] = '\0'; // always null-terminate
//        std::free(result);
//    }
//    else
//    {
//        // fallback: copy mangled name itself
//        std::strncpy(demangled_name, mangled_name, demangled_size - 1);
//        demangled_name[demangled_size - 1] = '\0'; // always null-terminate
//    }
}

void resetAll() noexcept
{
    // Prevent any qiti work while we reset everything
    // Demands exclusive lock, not a shared lock
    std::scoped_lock<qiti::ReentrantSharedMutex> lock(qiti_global_lock);
    
    getFunctionMap().clear();
    
    instrument::resetInstrumentation();
    profile::resetProfiling();
}
} // namespace qiti

extern "C" void QITI_API // Mark “no-instrument” to prevent recursing into itself
__cyg_profile_func_enter(void* this_fn, [[maybe_unused]] void* call_site) noexcept
{
    static thread_local int recursionCheck = 0;
    ++recursionCheck;
    assert(recursionCheck == 1);
    
    if (qiti::profile::isProfilingFunction(this_fn))
    {
        std::scoped_lock<std::mutex> lock(qiti_lock);
        qiti::profile::updateFunctionDataOnEnter(this_fn);
    }
    
    --recursionCheck;
}

extern "C" void QITI_API // Mark “no-instrument” to prevent recursing into itself
__cyg_profile_func_exit(void * this_fn, [[maybe_unused]] void* call_site) noexcept
{
    if (qiti::profile::isProfilingFunction(this_fn))
    {
        std::scoped_lock<std::mutex> lock(qiti_lock);
        qiti::profile::updateFunctionDataOnExit(this_fn);
    }
}
