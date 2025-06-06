
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

#include <qiti_Utils.hpp>

#include "qiti_include.hpp"
#include "qiti_Instrument.hpp"
#include "qiti_MallocHooks.hpp"

#include <cxxabi.h>
#include <dlfcn.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

//--------------------------------------------------------------------------

/** */
[[nodiscard]] static auto& getFunctionMap() noexcept
{
    static std::unordered_map<const void*, qiti::FunctionData> map;
    return map;
}

//--------------------------------------------------------------------------

namespace qiti
{
uint64_t Utils::getAllKnownFunctions(char* flatBuf,
                                     uint64_t maxFunctions,
                                     uint64_t maxNameLen) noexcept
{
    auto& map = getFunctionMap();
    const size_t count = std::min(map.size(),
                                  static_cast<size_t>(maxFunctions));
    
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

void* Utils::getAddressForMangledFunctionName(const char* mangledName) noexcept
{
    void* addr = dlsym(RTLD_DEFAULT, mangledName);
    return addr;
}

[[nodiscard]] qiti::FunctionData& Utils::getFunctionDataFromAddress(const void* functionAddress,
                                                                    const char* functionName,
                                                                    int functionType) noexcept
{
    auto& g_functionMap = getFunctionMap();
    
    auto it = g_functionMap.find(functionAddress);
    if (it == g_functionMap.end()) // not yet added to map
    {
        qiti::FunctionData data(functionAddress,
                                functionName,
                                static_cast<FunctionData::FunctionType>(functionType));
        
        auto [insertedIt, success] = g_functionMap.emplace(functionAddress, std::move(data));
        
        return insertedIt->second;
    }
    
    return it->second;
}

[[nodiscard]] const qiti::FunctionData* Utils::getFunctionData(const char* demangledFunctionName) noexcept
{
    auto& g_functionMap = getFunctionMap();
    
    auto it = std::find_if(g_functionMap.begin(), g_functionMap.end(),
                           [demangledFunctionName](const std::pair<const void*, const qiti::FunctionData&>& pair)
                           {
        return pair.second.getFunctionName() == std::string(demangledFunctionName);
    });
    
    if (it == g_functionMap.end()) // function not found
        return nullptr;
    
    return &(it->second);
}

void Utils::demangle(const char* mangled_name, char* demangled_name, uint64_t demangled_size) noexcept
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

void Utils::resetAll() noexcept
{
    getFunctionMap().clear();
    
    Instrument::resetInstrumentation();
    Profile::resetProfiling();
}

} // namespace qiti
