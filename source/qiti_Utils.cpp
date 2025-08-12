
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_Utils.cpp
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
#include "qiti_LockData.hpp"
#include "qiti_MallocHooks.hpp"

#include <cxxabi.h>
#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#else
#include <dlfcn.h>
#endif

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
#include <vector>

//--------------------------------------------------------------------------

#ifdef _WIN32
// Windows implementation of dladdr
int dladdr(const void* addr, Dl_info* info)
{
    if (!info) return 0;
    
    HMODULE hModule;
    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
                            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            (LPCSTR)addr, &hModule))
    {
        return 0;
    }
    
    // Get module filename
    static char moduleFilename[MAX_PATH];
    if (!GetModuleFileNameA(hModule, moduleFilename, MAX_PATH))
    {
        return 0;
    }
    
    info->dli_fname = moduleFilename;
    info->dli_fbase = hModule;
    info->dli_sname = nullptr;  // Symbol name lookup would require DbgHelp
    info->dli_saddr = nullptr;
    
    return 1;
}
#endif

/** */
[[nodiscard]] static auto& getFunctionMap() noexcept
{
    static std::unordered_map<const void*, qiti::FunctionData> map;
    return map;
}

/** */
[[nodiscard]] static const char* getFunctionName(const void* this_fn) noexcept
{
    Dl_info info;
    const char* functionName = nullptr;
    // Prevent memory leaks, clean up dynamically allocated function name strings
    static std::vector<std::unique_ptr<char>> names;
    
    if (dladdr(this_fn, &info) && info.dli_sname)
    {
        // Demangle the function name
        int status;
        char* demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
        if (status == 0 && demangled)
        {
            auto& ref = names.emplace_back(std::unique_ptr<char>(demangled));
            functionName = ref.get();
        }
        else
            functionName = info.dli_sname; // mangled name
    }
    
    return functionName;
}

//--------------------------------------------------------------------------

namespace qiti
{
void* Utils::getAddressForMangledFunctionName(const char* mangledName) noexcept
{
    void* addr = dlsym(RTLD_DEFAULT, mangledName);
    return addr;
}

[[nodiscard]] qiti::FunctionData& Utils::getFunctionDataFromAddress(const void* functionAddress,
                                                                    const char* functionName,
                                                                    int functionTypeInt) noexcept
{
    auto& g_functionMap = getFunctionMap();
    
    auto it = g_functionMap.find(functionAddress);
    if (it == g_functionMap.end()) // not yet added to map
    {
        if (functionName == nullptr)
            functionName = getFunctionName(functionAddress);
        
        auto functionType = qiti::FunctionData::FunctionType::unknown; // default to unknown
        if (functionTypeInt != -1)
            functionType = static_cast<FunctionData::FunctionType>(functionTypeInt);
        else if (functionName != nullptr)
            functionType = qiti::FunctionData::getFunctionType(functionName);

        qiti::FunctionData data(functionAddress,
                                functionName,
                                functionType);
        
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

std::vector<const qiti::FunctionData*> Utils::getAllFunctionData() noexcept
{
    std::vector<const qiti::FunctionData*> output;
    
    auto& functionMap = getFunctionMap();
    output.reserve(functionMap.size());
    for (auto& entry : functionMap)
        output.push_back(&entry.second);
    
    return output;
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
    LockData::resetAllListeners();
}

} // namespace qiti
