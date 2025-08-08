
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
    : pImpl(std::make_unique<Impl>())
{
    MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;
    
    pImpl->address = functionAddress;
    pImpl->functionType = functionType;
    if (functionName != nullptr)
        pImpl->functionName = functionName;
}

FunctionData::~FunctionData() noexcept = default;

FunctionData::Impl*       FunctionData::getImpl()       noexcept { return pImpl.get(); }
const FunctionData::Impl* FunctionData::getImpl() const noexcept { return pImpl.get(); }

FunctionData::FunctionData(FunctionData&& other) noexcept
    : pImpl(std::move(other.pImpl))
{
    qiti::ScopedNoHeapAllocations noAlloc;
}

FunctionData& FunctionData::operator=(FunctionData&& other) noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    if (this != &other)
    {
        pImpl = std::move(other.pImpl);
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

uint64_t FunctionData::getMinTimeSpentInFunctionCpu_ns() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->minTimeSpentInFunctionNanosecondsCpu;
}

uint64_t FunctionData::getMaxTimeSpentInFunctionCpu_ns() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->maxTimeSpentInFunctionNanosecondsCpu;
}

uint64_t FunctionData::getMinTimeSpentInFunctionWallClock_ns() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->minTimeSpentInFunctionNanosecondsWallClock;
}

uint64_t FunctionData::getMaxTimeSpentInFunctionWallClock_ns() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->maxTimeSpentInFunctionNanosecondsWallClock;
}

FunctionCallData FunctionData::getLastFunctionCall() const noexcept
{
    MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;
    
    return getImpl()->lastCallData;
}

std::vector<const FunctionData*> FunctionData::getAllProfiledFunctionData() noexcept
{
    MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;
    
    return Utils::getAllFunctionData();
}

std::vector<const FunctionData*> FunctionData::getCallers() const noexcept
{
    MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;
    
    const auto& callersSet = getImpl()->callers;
    std::vector<const FunctionData*> result;
    result.reserve(callersSet.size());
    
    for (const auto* caller : callersSet)
        result.push_back(caller);
    
    return result;
}

uint64_t FunctionData::getNumExceptionsThrown() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->numExceptionsThrown;
}

bool FunctionData::isConstructor() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    const auto type = getImpl()->functionType;
    return type == FunctionType::constructor ||
           type == FunctionType::copyConstructor ||
           type == FunctionType::moveConstructor;
}

bool FunctionData::isRegularConstructor() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->functionType == FunctionType::constructor;
}

bool FunctionData::isCopyConstructor() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->functionType == FunctionType::copyConstructor;
}

bool FunctionData::isMoveConstructor() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->functionType == FunctionType::moveConstructor;
}

bool FunctionData::isAssignment() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    const auto type = getImpl()->functionType;
    return type == FunctionType::copyAssignment ||
           type == FunctionType::moveAssignment;
}

bool FunctionData::isCopyAssignment() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->functionType == FunctionType::copyAssignment;
}

bool FunctionData::isMoveAssignment() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->functionType == FunctionType::moveAssignment;
}

bool FunctionData::isDestructor() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->functionType == FunctionType::destructor;
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
