
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

#include <cxxabi.h>       // __cxa_demangle
#include <execinfo.h>     // backtrace(), backtrace_symbols()
#include <dlfcn.h>        // dladdr()

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

thread_local bool qiti::MallocHooks::bypassMallocHooks = false;
thread_local uint32_t qiti::MallocHooks::numHeapAllocationsOnCurrentThread = 0;
thread_local uint64_t qiti::MallocHooks::totalAmountHeapAllocatedOnCurrentThread = 0;
thread_local uint64_t qiti::MallocHooks::currentAmountHeapAllocatedOnCurrentThread = 0;;
thread_local std::function<void()> qiti::MallocHooks::onNextHeapAllocation = nullptr;

// Thread-local allocation tracking for leak detection
static thread_local std::unordered_map<void*, std::size_t> g_allocationSizes;

static thread_local struct AllocationSizesCleanup final
{
    ~AllocationSizesCleanup() noexcept
    {
        qiti::MallocHooks::ScopedBypassMallocHooks bypassHooks;
        g_allocationSizes.clear(); // delete everything without triggering hooks
    }
} g_allocationSizesCleanup;

/** Functions we never want to count towards heap allocations that we track. */
static inline const std::array<const char*, 1> blackListedFunctions
{
    "Catch::Section::Section" // every time a Catch2 unit test enters a SECTION
};

/** Demangle a C++ ABI symbol name, or return the original on error */
static QITI_API_INTERNAL std::string demangle(const char* name) noexcept
{
    int status = 0;
    std::unique_ptr<char,void(*)(void*)> demangled
    {
        abi::__cxa_demangle(name, nullptr, nullptr, &status),
        std::free
    };
    return (status == 0 && demangled) ? demangled.get() : name;
}

/** Capture the current call stack (skipping the first `skip` frames) */
static QITI_API_INTERNAL std::vector<std::string> captureStackTrace(int framesToSkip = 1) noexcept
{
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
}

/** Check if the stack trace contains a frame whose demangled name contains the substring `funcName */
inline static QITI_API_INTERNAL bool stackContainsFunction(const std::string& funcName, int framesToSkip = 1) noexcept
{
    auto trace = captureStackTrace(framesToSkip);
    for (auto& frame : trace)
        if (frame.find(funcName) != std::string::npos)
            return true;
    return false;
}

/** */
[[maybe_unused]] inline static QITI_API_INTERNAL bool stackContainsBlacklistedFunction() noexcept
{
    qiti::MallocHooks::ScopedBypassMallocHooks bypassHooks;
    
    for (const char* func : blackListedFunctions)
        if (stackContainsFunction(std::string(func), /*framesToSkip*/ 4))
            return true;
    return false;
}

//--------------------------------------------------------------------------

void QITI_API_INTERNAL qiti::MallocHooks::mallocHook(std::size_t size) noexcept
{
    if (! isQitiTestRunning())
        return;
    
    if (qiti::MallocHooks::bypassMallocHooks)
        return;
    
    // Temporarily disable stack checking to debug Linux TSan issues
    // if (stackContainsBlacklistedFunction())
    //     return;
    
    ++numHeapAllocationsOnCurrentThread;
    totalAmountHeapAllocatedOnCurrentThread += size;
    currentAmountHeapAllocatedOnCurrentThread += size;

    if (onNextHeapAllocation != nullptr)
    {
        onNextHeapAllocation();
        onNextHeapAllocation = nullptr;
    }
}

[[maybe_unused]] static void QITI_API_INTERNAL freeHook(void* ptr) noexcept
{
    if (! isQitiTestRunning())
        return;
    
    if (qiti::MallocHooks::bypassMallocHooks)
        return;
    
    if (ptr != nullptr)
    {
        // Use map-based tracking (consistent with other implementations)
        auto it = g_allocationSizes.find(ptr);
        if (it != g_allocationSizes.end())
        {
            qiti::MallocHooks::ScopedBypassMallocHooks bypassHooks;
            qiti::MallocHooks::currentAmountHeapAllocatedOnCurrentThread -= it->second;
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
    if (! qiti::MallocHooks::bypassMallocHooks && ptr != nullptr)
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
    if (! qiti::MallocHooks::bypassMallocHooks && g_allocationSizes.size() > 0)
    {
        auto it = g_allocationSizes.find(ptr);
        if (it != g_allocationSizes.end())
        {
            qiti::MallocHooks::ScopedBypassMallocHooks bypassHooks;
            currentAmountHeapAllocatedOnCurrentThread -= it->second;
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
            currentAmountHeapAllocatedOnCurrentThread -= it->second;
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
        currentAmountHeapAllocatedOnCurrentThread += newSize;
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

#if defined(__APPLE__) || ! defined(QITI_ENABLE_THREAD_SANITIZER)
// macOS operator new/delete overrides for leak detection

void* QITI_API operator new(std::size_t size)
{
    void* ptr = std::malloc(size);
    if (ptr == nullptr)
        throw std::bad_alloc{};
    
    if (! qiti::MallocHooks::bypassMallocHooks)
        qiti::MallocHooks::mallocHookWithTracking(ptr, size);
    
    return ptr;
}

void* QITI_API operator new[](std::size_t size)
{
    void* ptr = std::malloc(size);
    if (ptr == nullptr)
        throw std::bad_alloc{};
    
    if (! qiti::MallocHooks::bypassMallocHooks)
        qiti::MallocHooks::mallocHookWithTracking(ptr, size);
    
    return ptr;
}

void QITI_API operator delete(void* ptr) noexcept
{
    if (ptr != nullptr)
    {
        if (! qiti::MallocHooks::bypassMallocHooks)
            qiti::MallocHooks::freeHookWithTracking(ptr);
        std::free(ptr);
    }
}

void QITI_API operator delete[](void* ptr) noexcept
{
    if (ptr != nullptr)
    {
        if (! qiti::MallocHooks::bypassMallocHooks)
            qiti::MallocHooks::freeHookWithTracking(ptr);
        std::free(ptr);
    }
}

// Sized delete operators (C++14)
void QITI_API operator delete(void* ptr, std::size_t /*size*/) noexcept
{
    if (ptr != nullptr)
    {
        if (! qiti::MallocHooks::bypassMallocHooks)
            qiti::MallocHooks::freeHookWithTracking(ptr);
        std::free(ptr);
    }
}

void QITI_API operator delete[](void* ptr, std::size_t /*size*/) noexcept
{
    if (ptr != nullptr)
    {
        if (! qiti::MallocHooks::bypassMallocHooks)
            qiti::MallocHooks::freeHookWithTracking(ptr);
        std::free(ptr);
    }
}
#endif // defined(__APPLE__) || ! defined(QITI_ENABLE_THREAD_SANITIZER)

//--------------------------------------------------------------------------



//--------------------------------------------------------------------------
