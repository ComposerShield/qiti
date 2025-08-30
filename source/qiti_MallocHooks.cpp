
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_MallocHooks.cpp
 *
 * @author   Adam Shield
 * @date     2025-05-25
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "qiti_MallocHooks.hpp"

#include "qiti_Utils.hpp"

#ifdef _WIN32
  #include <windows.h>
  #include <dbghelp.h>
#else
  #include <cxxabi.h>     // __cxa_demangle
  #include <dlfcn.h>      // dladdr()
  #include <execinfo.h>   // backtrace(), backtrace_symbols()
#endif

#include <array>
#include <memory>
#include <mutex>
#include <sstream>        // std::ostringstream
#include <string>
#include <unordered_map>
#include <vector>

//--------------------------------------------------------------------------

extern bool isQitiTestRunning() noexcept;

//--------------------------------------------------------------------------

// Thread-local malloc hook state variables
static thread_local bool g_bypassMallocHooks = false;
static thread_local uint32_t g_numHeapAllocationsOnCurrentThread = 0;
static thread_local uint64_t g_totalAmountHeapAllocatedOnCurrentThread = 0;
static thread_local uint64_t g_currentAmountHeapAllocatedOnCurrentThread = 0;
static thread_local std::function<void()> g_onNextHeapAllocation = nullptr;

// Accessor function implementations
bool& qiti::MallocHooks::getBypassMallocHooks() noexcept
{
    return g_bypassMallocHooks;
}

uint32_t& qiti::MallocHooks::getNumHeapAllocationsOnCurrentThread() noexcept
{
    return g_numHeapAllocationsOnCurrentThread;
}

uint64_t& qiti::MallocHooks::getTotalAmountHeapAllocatedOnCurrentThread() noexcept
{
    return g_totalAmountHeapAllocatedOnCurrentThread;
}

uint64_t& qiti::MallocHooks::getCurrentAmountHeapAllocatedOnCurrentThread() noexcept
{
    return g_currentAmountHeapAllocatedOnCurrentThread;
}

std::function<void()>& qiti::MallocHooks::getOnNextHeapAllocation() noexcept
{
    return g_onNextHeapAllocation;
}

// Thread-local allocation tracking for leak detection
static thread_local std::unordered_map<void*, std::size_t> g_allocationSizes;

#ifndef _WIN32
static thread_local struct AllocationSizesCleanup final
{
    QITI_API_INTERNAL ~AllocationSizesCleanup() noexcept
    {
        qiti::MallocHooks::ScopedBypassMallocHooks bypassHooks;
        g_allocationSizes.clear(); // delete everything without triggering hooks
    }
} g_allocationSizesCleanup;
#endif

/** Functions we never want to count towards heap allocations that we track. */
static inline const std::array<const char*, 1> blackListedFunctions
{
    "Catch::Section::Section" // every time a Catch2 unit test enters a SECTION
};

/** Demangle a C++ ABI symbol name, or return the original on error */
[[maybe_unused]] QITI_API_INTERNAL static std::string demangle(const char* name) noexcept
{
#ifdef _WIN32
    // Windows: Use UnDecorateSymbolName
    char buffer[1024];
    if (UnDecorateSymbolName(name, buffer, sizeof(buffer), UNDNAME_COMPLETE))
    {
        return std::string(buffer);
    }
    return std::string(name);
#else
    int status = 0;
    std::unique_ptr<char,void(*)(void*)> demangled
    {
        abi::__cxa_demangle(name, nullptr, nullptr, &status),
        std::free
    };
    return (status == 0 && demangled) ? demangled.get() : name;
#endif
}

/** Capture the current call stack (skipping the first `skip` frames) */
QITI_API_INTERNAL static std::vector<std::string> captureStackTrace(int framesToSkip = 1) noexcept
{
#ifdef _WIN32
#if 0 // Temporarily disabled for debugging Windows initialization issues
    constexpr int MAX_FRAMES = 128;
    void* stack[MAX_FRAMES];
    WORD frames = CaptureStackBackTrace(static_cast<DWORD>(framesToSkip), MAX_FRAMES - framesToSkip, stack, nullptr);
    
    std::vector<std::string> out;
    out.reserve(frames);
    
    HANDLE process = GetCurrentProcess();
    BOOL symInitResult = SymInitialize(process, nullptr, TRUE);
    
    if (symInitResult)
    {
        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        auto symbol = reinterpret_cast<PSYMBOL_INFO>(buffer);
        symbol->MaxNameLen = MAX_SYM_NAME;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        
        for (int i = 0; i < frames; ++i)
        {
            auto address = reinterpret_cast<DWORD64>(stack[i]);
            if (SymFromAddr(process, address, nullptr, symbol))
            {
                out.push_back(demangle(symbol->Name));
            }
            else
            {
                std::ostringstream oss;
                oss << stack[i];
                out.push_back(oss.str());
            }
        }
        
        SymCleanup(process);
    }
    else
    {
        // Fallback: just show addresses if symbol initialization fails
        for (int i = 0; i < frames; ++i)
        {
            std::ostringstream oss;
            oss << stack[i];
            out.push_back(oss.str());
        }
    }
    return out;
#else
    (void)framesToSkip;
    // Windows: Disable stack tracing temporarily for debugging
    std::vector<std::string> out;
    out.push_back("<stack tracing disabled on Windows>");
    return out;
#endif
#else
    constexpr int MAX_FRAMES = 128;
    void* addrs[MAX_FRAMES];
    int frames = backtrace(addrs, MAX_FRAMES);

    std::vector<std::string> out;
    out.reserve(static_cast<size_t>(frames - framesToSkip));

    for (int i = framesToSkip; i < frames; ++i)
    {
        Dl_info info;
        if (dladdr(addrs[i], &info) && info.dli_sname)
        {
            // demangle the raw symbol
            out.push_back(demangle(info.dli_sname));
        }
        else
        {
            // fallback: just print the address
            std::ostringstream oss;
            oss << addrs[i];
            out.push_back(oss.str());
        }
    }
    return out;
#endif
}

/** Check if the stack trace contains a frame whose demangled name contains the substring `funcName */
[[nodiscard]] QITI_API_INTERNAL inline static bool stackContainsFunction(const std::string& funcName, int framesToSkip = 1) noexcept
{
    auto trace = captureStackTrace(framesToSkip);
    for (auto& frame : trace)
        if (frame.find(funcName) != std::string::npos)
            return true;
    return false;
}

/** */
QITI_API_INTERNAL inline static bool stackContainsBlacklistedFunction() noexcept
{
    qiti::MallocHooks::ScopedBypassMallocHooks bypassHooks;
    
    for (const char* func : blackListedFunctions)
        if (stackContainsFunction(std::string(func), /*framesToSkip*/ 4))
            return true;
    return false;
}

//--------------------------------------------------------------------------

QITI_API_INTERNAL void qiti::MallocHooks::mallocHook(std::size_t size) noexcept
{
    if (! isQitiTestRunning())
        return;
    
    if (g_bypassMallocHooks)
        return;
    
    if (stackContainsBlacklistedFunction())
        return;
    
    ++g_numHeapAllocationsOnCurrentThread;
    g_totalAmountHeapAllocatedOnCurrentThread += size;
    g_currentAmountHeapAllocatedOnCurrentThread += size;

    if (g_onNextHeapAllocation != nullptr)
    {
        g_onNextHeapAllocation();
        g_onNextHeapAllocation = nullptr;
    }
}

[[maybe_unused]] QITI_API_INTERNAL static void freeHook(void* ptr) noexcept
{
    if (! isQitiTestRunning())
        return;
    
    if (g_bypassMallocHooks)
        return;
    
    if (ptr != nullptr)
    {
        // Use map-based tracking (consistent with other implementations)
        auto it = g_allocationSizes.find(ptr);
        if (it != g_allocationSizes.end())
        {
            qiti::MallocHooks::ScopedBypassMallocHooks bypassHooks;
            g_currentAmountHeapAllocatedOnCurrentThread -= it->second;
            g_allocationSizes.erase(it); // deletes
        }
    }
}

void qiti::MallocHooks::mallocHookWithTracking(void* ptr, std::size_t size) noexcept
{
    if (! isQitiTestRunning())
        return;
        
    // Always call the basic malloc hook (it has its own bypass check)
    mallocHook(size);
    
    // Only do leak tracking if we're not bypassing and have a valid pointer
    if (! g_bypassMallocHooks && ptr != nullptr)
    {
        qiti::MallocHooks::ScopedBypassMallocHooks bypassHooks;
        g_allocationSizes[ptr] = size; // heap allocates
    }
}

void qiti::MallocHooks::freeHookWithTracking(void* ptr) noexcept
{
    if (! isQitiTestRunning() || ptr == nullptr)
        return;
    
    // Only do leak tracking if we're not bypassing
    if (! g_bypassMallocHooks && g_allocationSizes.size() > 0)
    {
        auto it = g_allocationSizes.find(ptr);
        if (it != g_allocationSizes.end())
        {
            qiti::MallocHooks::ScopedBypassMallocHooks bypassHooks;
            g_currentAmountHeapAllocatedOnCurrentThread -= it->second;
            g_allocationSizes.erase(it); // deletes
        }
    }
}

void qiti::MallocHooks::reallocHookWithTracking(void* oldPtr, void* newPtr, std::size_t oldSize, std::size_t newSize) noexcept
{
    if (! isQitiTestRunning())
        return;
        
    // Handle the old allocation
    if (oldPtr != nullptr)
    {
        auto it = g_allocationSizes.find(oldPtr);
        if (it != g_allocationSizes.end())
        {
            qiti::MallocHooks::ScopedBypassMallocHooks bypassHooks;
            g_currentAmountHeapAllocatedOnCurrentThread -= it->second;
            g_allocationSizes.erase(it); // deletes
        }
    }
    
    // Handle the new allocation
    if (newPtr != nullptr)
    {
        // Only call mallocHook for the net size change
        if (newSize > oldSize)
        {
            mallocHook(newSize - oldSize);
        }
        
        qiti::MallocHooks::ScopedBypassMallocHooks bypassHooks;
        g_currentAmountHeapAllocatedOnCurrentThread += newSize;
        g_allocationSizes[newPtr] = newSize; // heap allocates
    }
}

//--------------------------------------------------------------------------

/**
 Memory allocation hook implementation:
 - macOS: Uses malloc interposition (implemented below)
 - Linux with ThreadSanitizer: Uses __sanitizer_malloc_hook (in qiti_tests_client.cpp)  
 - Linux without ThreadSanitizer: Uses malloc hooks (to be implemented)
 
 Note: No operator new/delete overrides needed since all platforms use malloc-level hooks.
 */

//--------------------------------------------------------------------------

#if (defined(__APPLE__) || ! defined(QITI_ENABLE_CLANG_THREAD_SANITIZER)) && ! defined(_WIN32)
// macOS/Linux operator new/delete overrides for leak detection
// Windows overrides are in qiti_client_executable.cpp (executable-side)

QITI_API void* operator new(std::size_t size)
{
    void* ptr = std::malloc(size);
    if (ptr == nullptr)
        throw std::bad_alloc{};
    
    if (! g_bypassMallocHooks)
        qiti::MallocHooks::mallocHookWithTracking(ptr, size);
    
    return ptr;
}

QITI_API void* operator new[](std::size_t size)
{
    void* ptr = std::malloc(size);
    if (ptr == nullptr)
        throw std::bad_alloc{};
    
    if (! g_bypassMallocHooks)
        qiti::MallocHooks::mallocHookWithTracking(ptr, size);
    
    return ptr;
}

QITI_API void operator delete(void* ptr) noexcept
{
    if (ptr != nullptr)
    {
        if (! g_bypassMallocHooks)
            qiti::MallocHooks::freeHookWithTracking(ptr);
        std::free(ptr);
    }
}

QITI_API void operator delete[](void* ptr) noexcept
{
    if (ptr != nullptr)
    {
        if (! g_bypassMallocHooks)
            qiti::MallocHooks::freeHookWithTracking(ptr);
        std::free(ptr);
    }
}

// Sized delete operators (C++14)
QITI_API void operator delete(void* ptr, std::size_t /*size*/) noexcept
{
    if (ptr != nullptr)
    {
        if (! g_bypassMallocHooks)
            qiti::MallocHooks::freeHookWithTracking(ptr);
        std::free(ptr);
    }
}

QITI_API void operator delete[](void* ptr, std::size_t /*size*/) noexcept
{
    if (ptr != nullptr)
    {
        if (! g_bypassMallocHooks)
            qiti::MallocHooks::freeHookWithTracking(ptr);
        std::free(ptr);
    }
}
#endif // (defined(__APPLE__) || ! defined(QITI_ENABLE_CLANG_THREAD_SANITIZER)) && ! defined(_WIN32)

//--------------------------------------------------------------------------



//--------------------------------------------------------------------------
