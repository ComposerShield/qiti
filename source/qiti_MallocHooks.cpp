
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_LockData.hpp
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
#include <sstream>        // std::ostringstream
#include <string>
#include <vector>

//--------------------------------------------------------------------------

extern bool isQitiTestRunning() noexcept;

//--------------------------------------------------------------------------

thread_local bool qiti::MallocHooks::bypassMallocHooks = false;
thread_local uint32_t qiti::MallocHooks::numHeapAllocationsOnCurrentThread = 0;
thread_local uint64_t qiti::MallocHooks::amountHeapAllocatedOnCurrentThread;
thread_local std::function<void()> qiti::MallocHooks::onNextHeapAllocation = nullptr;

/** Functions we never want to count towards heap allocations that we track. */
static inline const std::array<std::string, 1> blackListedFunctions
{
    "Catch::Section::Section" // every time a Catch2 unit test enters a SECTION
};

/** Demangle a C++ ABI symbol name, or return the original on error */
static std::string demangle(const char* name) noexcept
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
static std::vector<std::string> captureStackTrace(int framesToSkip = 1) noexcept
{
    qiti::MallocHooks::ScopedBypassMallocHooks bypassHooks;
    
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
inline static bool stackContainsFunction(const std::string& funcName, int framesToSkip = 1) noexcept
{
    auto trace = captureStackTrace(framesToSkip);
    for (auto& frame : trace)
        if (frame.find(funcName) != std::string::npos)
            return true;
    return false;
}

/** */
inline static bool stackContainsBlacklistedFunction() noexcept
{
    /** Only one thread accesses blackListedFunctions at a time. */
    static std::mutex stackContainsBlacklistedFunctionMutex{};
    std::scoped_lock lock(stackContainsBlacklistedFunctionMutex);
    
    for (const auto& func : blackListedFunctions)
        if (stackContainsFunction(func, /*framesToSkip*/ 4))
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
    amountHeapAllocatedOnCurrentThread += size;

    if (onNextHeapAllocation != nullptr)
    {
        onNextHeapAllocation();
        onNextHeapAllocation = nullptr;
    }
}

//--------------------------------------------------------------------------

#if defined(__APPLE__)
/**
 Due to differences in the Thread Sanitizer runtime on Apple vs. Linux,
 we need to insert our logic into "operator new" in macOS from the Qiti
 dylib but on Linux, we must insert it into the malloc hook provided by
 Thread Sanitizer directly in the final executable (qiti_tests_client.cpp).
 */
void* operator new(std::size_t size)
{
    qiti::MallocHooks::mallocHook(size);
    
    // Original implementation
    if (void* ptr = std::malloc(size))
        return ptr;
    
    throw std::bad_alloc{};
}

void* operator new[](std::size_t size)
{
    qiti::MallocHooks::mallocHook(size);
    
    if (size == 0)
        ++size; // avoid std::malloc(0) which may return nullptr on success
 
    if (void* ptr = std::malloc(size))
        return ptr;
 
    throw std::bad_alloc{};
}
#endif // defined(__APPLE__)

//--------------------------------------------------------------------------
