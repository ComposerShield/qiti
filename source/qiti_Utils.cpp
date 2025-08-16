
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

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#else
#include <cxxabi.h>
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
                            reinterpret_cast<LPCSTR>(addr), &hModule))
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
    info->dli_sname = nullptr;
    info->dli_saddr = nullptr;
    
    // Try to get symbol name using DbgHelp
    static char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
    PSYMBOL_INFO pSymbol = reinterpret_cast<PSYMBOL_INFO>(buffer);
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymbol->MaxNameLen = MAX_SYM_NAME;
    
    DWORD64 displacement = 0;
    HANDLE hProcess = GetCurrentProcess();
    
    // Initialize symbols if not already done
    static bool symInitialized = false;
    if (! symInitialized)
    {
        // Set symbol search path and options for better symbol resolution
        SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_INCLUDE_32BIT_MODULES);
        SymInitialize(hProcess, nullptr, TRUE);
        symInitialized = true;
    }
    
    if (SymFromAddr(hProcess, reinterpret_cast<DWORD64>(addr), &displacement, pSymbol))
    {
        // Store the symbol name - use a static buffer to keep it alive
        static char symbolNameBuffer[MAX_SYM_NAME];
        strncpy_s(symbolNameBuffer, sizeof(symbolNameBuffer), pSymbol->Name, _TRUNCATE);
        info->dli_sname = symbolNameBuffer;
        info->dli_saddr = reinterpret_cast<void*>(pSymbol->Address);
    }
    else
    {
        // Debug: Try to understand why symbol lookup failed
        DWORD error = GetLastError();
        // For now, just ensure dli_sname remains nullptr
        info->dli_sname = nullptr;
    }
    
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
    
    #ifdef _WIN32
    // Debug output for Windows
    static int debugCount = 0;
    if (debugCount < 5) // Only print first 5 calls to avoid spam
    {
        printf("DEBUG getFunctionName: addr=%p\n", this_fn);
        debugCount++;
    }
    #endif
    
    if (dladdr(this_fn, &info) && info.dli_sname)
    {
        #ifdef _WIN32
        if (debugCount <= 5)
        {
            printf("DEBUG getFunctionName: Found symbol '%s'\n", info.dli_sname);
        }
        #endif
        // Demangle the function name
#ifdef _WIN32
        // Windows: Use UnDecorateSymbolName
        static char buffer[1024];
        if (UnDecorateSymbolName(info.dli_sname, buffer, sizeof(buffer), UNDNAME_COMPLETE))
        {
            functionName = buffer;
        }
        else
            functionName = info.dli_sname; // mangled name
#else
        int status;
        char* demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
        if (status == 0 && demangled)
        {
            auto& ref = names.emplace_back(std::unique_ptr<char>(demangled));
            functionName = ref.get();
        }
        else
            functionName = info.dli_sname; // mangled name
#endif
    }
    else
    {
        #ifdef _WIN32
        if (debugCount <= 5)
        {
            printf("DEBUG getFunctionName: dladdr failed or no symbol name for addr=%p\n", this_fn);
        }
        #endif
    }
    
    return functionName;
}

//--------------------------------------------------------------------------

namespace qiti
{
void* Utils::getAddressForMangledFunctionName(const char* mangledName) noexcept
{
#ifdef _WIN32
    // Windows: Use GetProcAddress with current module
    HMODULE hModule = GetModuleHandleA(nullptr); // Current executable
    return reinterpret_cast<void*>(GetProcAddress(hModule, mangledName));
#else
    void* addr = dlsym(RTLD_DEFAULT, mangledName);
    return addr;
#endif
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
#ifdef _WIN32
    // Windows: Use UnDecorateSymbolName
    if (UnDecorateSymbolName(mangled_name, demangled_name, static_cast<DWORD>(demangled_size), UNDNAME_COMPLETE))
    {
        // Success, demangled_name is already filled
    }
    else
    {
        // fallback: copy mangled name itself
        strncpy_s(demangled_name, demangled_size, mangled_name, _TRUNCATE);
    }
#else
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
#endif
}

void Utils::resetAll() noexcept
{
    getFunctionMap().clear();
    
    Instrument::resetInstrumentation();
    Profile::resetProfiling();
    LockData::resetAllListeners();
}

} // namespace qiti
