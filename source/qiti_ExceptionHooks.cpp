/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_ExceptionHooks.cpp
 *
 * @author   Adam Shield
 * @date     2025-05-16
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "qiti_FunctionCallData.hpp"
#include "qiti_FunctionCallData_Impl.hpp"
#include "qiti_FunctionData.hpp"
#include "qiti_FunctionData_Impl.hpp"
#include "qiti_Profile.hpp"
#include "qiti_ScopedNoHeapAllocations.hpp"

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#else
#include <dlfcn.h>
#endif

#include <typeinfo>

//--------------------------------------------------------------------------

extern "C"
{
    // Function pointers to original exception handling functions
    static void (*original_cxa_throw)(void*, std::type_info*, void(*)(void*)) = nullptr;
    static void* (*original_cxa_begin_catch)(void*) = nullptr;
    static void (*original_cxa_end_catch)() = nullptr;
}


// Initialize function pointers to original exception functions
QITI_API_INTERNAL static void initializeExceptionHooks() noexcept
{
#ifdef _WIN32
    // Windows: Get exception functions from the C++ runtime
    HMODULE msvcrt = GetModuleHandleA("msvcr120.dll"); // Or try other versions
    if (!msvcrt) msvcrt = GetModuleHandleA("msvcr110.dll");
    if (!msvcrt) msvcrt = GetModuleHandleA("msvcr100.dll");
    if (!msvcrt) msvcrt = GetModuleHandleA("ucrtbase.dll");
    
    if (msvcrt && !original_cxa_throw)
    {
        original_cxa_throw = reinterpret_cast<void(*)(void*, std::type_info*, void(*)(void*))>(
            GetProcAddress(msvcrt, "__cxa_throw"));
    }
    if (msvcrt && !original_cxa_begin_catch)
    {
        original_cxa_begin_catch = reinterpret_cast<void*(*)(void*)>(
            GetProcAddress(msvcrt, "__cxa_begin_catch"));
    }
    if (msvcrt && !original_cxa_end_catch)
    {
        original_cxa_end_catch = reinterpret_cast<void(*)()>(
            GetProcAddress(msvcrt, "__cxa_end_catch"));
    }
#else
    if (! original_cxa_throw)
    {
        original_cxa_throw = reinterpret_cast<void(*)(void*, std::type_info*, void(*)(void*))>(
            dlsym(RTLD_NEXT, "__cxa_throw"));
    }
    if (! original_cxa_begin_catch)
    {
        original_cxa_begin_catch = reinterpret_cast<void*(*)(void*)>(
            dlsym(RTLD_NEXT, "__cxa_begin_catch"));
    }
    if (! original_cxa_end_catch)
    {
        original_cxa_end_catch = reinterpret_cast<void(*)()>(
            dlsym(RTLD_NEXT, "__cxa_end_catch"));
    }
#endif
}

//--------------------------------------------------------------------------

extern "C"
{

QITI_API void __cxa_throw(void* thrown_object, 
                         std::type_info* tinfo, 
                         void (*dest)(void*))
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    initializeExceptionHooks();
    
    // Record that the current function threw an exception
    if (! qiti::g_callStack.empty())
    {
        auto* currentFunc = qiti::g_callStack.top();
        if (currentFunc)
        {
            auto* impl = currentFunc->getImpl();
            impl->numExceptionsThrown++;
            
            // Also mark the current call as having thrown an exception
            auto* callImpl = impl->lastCallData.getImpl();
            callImpl->numExceptionsThrown++;
        }
    }
    
    // Call the original __cxa_throw to maintain normal exception behavior
    if (original_cxa_throw)
    {
        original_cxa_throw(thrown_object, tinfo, dest);
    }
}

QITI_API void* __cxa_begin_catch(void* exceptionObject)
{
    initializeExceptionHooks();
    
    // For now, just call the original function
    // We could add catch tracking here in the future
    if (original_cxa_begin_catch)
    {
        return original_cxa_begin_catch(exceptionObject);
    }
    return nullptr;
}

QITI_API void __cxa_end_catch()
{
    initializeExceptionHooks();
    
    // For now, just call the original function
    // We could add catch tracking here in the future  
    if (original_cxa_end_catch)
    {
        original_cxa_end_catch();
    }
}

} // extern "C"
