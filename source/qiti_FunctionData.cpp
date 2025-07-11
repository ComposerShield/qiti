
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_FunctionData.cpp
 *
 * @author   Adam Shield
 * @date     2025-05-16
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "qiti_FunctionData.hpp"

#include "qiti_FunctionData_Impl.hpp"
#include "qiti_MallocHooks.hpp"
#include "qiti_ScopedNoHeapAllocations.hpp"

#include <dlfcn.h>

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <utility>
#include <thread>
#include <vector>

//--------------------------------------------------------------------------

static constexpr size_t MAX_THREADS = qiti::FunctionData::Impl::MAX_THREADS;
static constexpr size_t THREAD_ID_NOT_FOUND   = MAX_THREADS;

// global array + counter for id→small-index mapping
static std::atomic<size_t>                  g_nextThreadIndex{0};
static std::array<std::thread::id, MAX_THREADS> g_threadIds;

// lookup only (returns NOT_FOUND if we've never seen this id)
inline static size_t threadIdToIndex(const std::thread::id& tid) noexcept
{
    size_t used = g_nextThreadIndex.load(std::memory_order_acquire);
    for (size_t i = 0; i < used; ++i)
        if (g_threadIds[i] == tid)
            return i;
    return THREAD_ID_NOT_FOUND;
}

// record & lookup: if unseen, atomically grab a new slot
inline static size_t threadIdToIndexOrRegister(const std::thread::id& tid) noexcept
{
    // check existing
    size_t used = g_nextThreadIndex.load(std::memory_order_acquire);
    for (size_t i = 0; i < used; ++i) {
        if (g_threadIds[i] == tid)
            return i;
    }
    // new thread -> register it
    size_t idx = g_nextThreadIndex.fetch_add(1, std::memory_order_acq_rel);
    assert(idx < MAX_THREADS && "Exceeded MAX_THREADS threads!");
    g_threadIds[idx] = tid;
    return idx;
}

//--------------------------------------------------------------------------

namespace qiti
{

FunctionData::FunctionData(const void* functionAddress,
                           const char* functionName,
                           FunctionType functionType) noexcept
{
    static_assert(sizeof(FunctionData::Impl)  <= FunctionData::ImplSize,
                  "Impl is too large for FunctionData::implStorage");
    static_assert(alignof(FunctionData::Impl) == FunctionData::ImplAlign,
                  "Impl alignment stricter than FunctionData::implStorage");
        
    MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;
    
    // Allocate Impl on the stack instead of the heap
    new (implStorage) Impl;
        
    auto* impl = getImpl();
    
    impl->address = functionAddress;
    impl->functionType = functionType;
    if (functionName != nullptr)
        impl->functionName = functionName; // else remain defaulted string
}

FunctionData::~FunctionData() noexcept
{
    getImpl()->~Impl();
}

FunctionData::Impl*       FunctionData::getImpl()       noexcept { return reinterpret_cast<Impl*>(implStorage); }
const FunctionData::Impl* FunctionData::getImpl() const noexcept { return reinterpret_cast<const Impl*>(implStorage); }

// Move constructor
FunctionData::FunctionData(FunctionData&& other) noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    // move‐construct into our storage
    new (implStorage) Impl(std::move(*other.getImpl()));
    // destroy their Impl so we can re‐init it
    other.getImpl()->~Impl();
    // default‐construct theirs back into a valid (empty) state
    new (other.implStorage) Impl();
}

// Move assignment
FunctionData& FunctionData::operator=(FunctionData&& other) noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    if (this != &other)
    {
        // destroy our current one
        getImpl()->~Impl();
        // move‐construct into our storage
        new (implStorage) Impl(std::move(*other.getImpl()));
        // tear down theirs…
        other.getImpl()->~Impl();
        // …and put them back into a default‐constructed safe state
        new (other.implStorage) Impl();
    }
    return *this;
}

const char* FunctionData::getFunctionName() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->functionName;
}

uint64_t FunctionData::getNumTimesCalled() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->numTimesCalled;
}

uint64_t FunctionData::getAverageTimeSpentInFunctionCpu_ns() const noexcept
{
    return getImpl()->averageTimeSpentInFunctionNanosecondsCpu;
}

uint64_t FunctionData::getAverageTimeSpentInFunctionWallClock_ns() const noexcept
{
    return getImpl()->averageTimeSpentInFunctionNanosecondsWallClock;
}

FunctionCallData FunctionData::getLastFunctionCall() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->lastCallData;
}

std::vector<const FunctionData*> FunctionData::getAllProfiledFunctionData() noexcept
{
    MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;
    
    return Utils::getAllFunctionData();
}

void FunctionData::functionCalled() noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    auto* impl = getImpl();
    ++impl->numTimesCalled;
    
    auto idx = threadIdToIndexOrRegister(std::this_thread::get_id());
    impl->threadsCalledOn.set(idx);
}

bool FunctionData::wasCalledOnThread(std::thread::id thread) const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    auto* impl = getImpl();
    auto idx = threadIdToIndex(thread);
    return (idx != THREAD_ID_NOT_FOUND)
           && impl->threadsCalledOn.test(idx);
}

void FunctionData::addListener(FunctionData::Listener* listener) noexcept
{
    getImpl()->listeners.insert(listener);
}

void FunctionData::removeListener(FunctionData::Listener* listener) noexcept
{
    getImpl()->listeners.erase(listener);
}

} // namespace qiti
