
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

// Store size in header before user pointer
struct AllocationHeader
{
    std::size_t size;
};

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
inline static QITI_API_INTERNAL bool stackContainsBlacklistedFunction() noexcept
{
    qiti::MallocHooks::ScopedBypassMallocHooks bypassHooks;
    
    for (const char* func : blackListedFunctions)
        if (stackContainsFunction(std::string(func), /*framesToSkip*/ 4))
            return true;
    return false;
}

//--------------------------------------------------------------------------

void qiti::MallocHooks::mallocHook(std::size_t size) noexcept
{
    if (! isQitiTestRunning())
        return;
    
    if (qiti::MallocHooks::bypassMallocHooks)
        return;
    
    if (stackContainsBlacklistedFunction())
        return;
    
    ++numHeapAllocationsOnCurrentThread;
    totalAmountHeapAllocatedOnCurrentThread += size;
    currentAmountHeapAllocatedOnCurrentThread += size;

    if (onNextHeapAllocation != nullptr)
    {
        onNextHeapAllocation();
        onNextHeapAllocation = nullptr;
    }
}

static void freeHook(void* ptr) noexcept
{
    if (! isQitiTestRunning())
        return;
    
    if (qiti::MallocHooks::bypassMallocHooks)
        return;
    
    if (ptr != nullptr)
    {
        // Extract size from header
        AllocationHeader* header = static_cast<AllocationHeader*>(ptr) - 1;
        currentAmountHeapAllocatedOnCurrentThread -= header->size;
    }
}

//--------------------------------------------------------------------------

#if defined(__APPLE__) || ! defined(QITI_ENABLE_THREAD_SANITIZER)
/**
 Memory allocation hook implementation:
 - macOS: Always uses operator new override
 - Linux with ThreadSanitizer: Uses __sanitizer_malloc_hook (in qiti_tests_client.cpp)
 - Linux without ThreadSanitizer: Uses operator new override (matches macOS implementation)
 */
QITI_API void* operator new(std::size_t size)
{
    qiti::MallocHooks::mallocHook(size);
    
    // Allocate extra space for header + user data
    std::size_t totalSize = sizeof(AllocationHeader) + size;
    void* rawPtr = std::malloc(totalSize);
    if (! rawPtr)
        throw std::bad_alloc{};
    
    // Store size in header
    AllocationHeader* header = static_cast<AllocationHeader*>(rawPtr);
    header->size = size;
    
    // Return pointer after header
    return header + 1;
}

QITI_API void* operator new[](std::size_t size)
{
    qiti::MallocHooks::mallocHook(size);
    
    if (size == 0)
        ++size; // avoid std::malloc(0) which may return nullptr on success
    
    // Allocate extra space for header + user data
    std::size_t totalSize = sizeof(AllocationHeader) + size;
    void* rawPtr = std::malloc(totalSize);
    if (! rawPtr)
        throw std::bad_alloc{};
    
    // Store size in header
    AllocationHeader* header = static_cast<AllocationHeader*>(rawPtr);
    header->size = size;
    
    // Return pointer after header
    return header + 1;
}
#endif // defined(__APPLE__) || ! defined(QITI_ENABLE_THREAD_SANITIZER)

QITI_API void operator delete(void* ptr) noexcept
{
    if (ptr != nullptr)
    {
        freeHook(ptr);
        
        // Get original allocation by backing up to header
        AllocationHeader* header = static_cast<AllocationHeader*>(ptr) - 1;
        std::free(header);
    }
}

QITI_API void operator delete[](void* ptr) noexcept
{
    if (ptr != nullptr)
    {
        freeHook(ptr);
        
        // Get original allocation by backing up to header
        AllocationHeader* header = static_cast<AllocationHeader*>(ptr) - 1;
        std::free(header);
    }
}

//--------------------------------------------------------------------------
