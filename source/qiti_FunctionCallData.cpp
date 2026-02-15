
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_FunctionCallData.cpp
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
#include "qiti_Profile.hpp"
#include "qiti_ScopedNoHeapAllocations.hpp"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <memory>
#include <utility>

//--------------------------------------------------------------------------

namespace qiti
{

FunctionCallData::FunctionCallData() noexcept
    : pImpl(std::make_unique<Impl>())
{
    qiti::ScopedNoHeapAllocations noAlloc;
}

FunctionCallData::~FunctionCallData() noexcept = default;

FunctionCallData::FunctionCallData(FunctionCallData&& other) noexcept
    : pImpl(std::move(other.pImpl))
{
    qiti::ScopedNoHeapAllocations noAlloc;
}

FunctionCallData& FunctionCallData::operator=(FunctionCallData&& other) noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    if (this != &other) {
        pImpl = std::move(other.pImpl);
    }
    return *this;
}

FunctionCallData::FunctionCallData(const FunctionCallData& other) noexcept
    : pImpl(std::make_unique<Impl>(*other.pImpl))
{
}

FunctionCallData& FunctionCallData::operator=(const FunctionCallData& other) noexcept
{
    if (this != &other)
    {
        pImpl = std::make_unique<Impl>(*other.pImpl);
    }
    return *this;
}

FunctionCallData::Impl*       FunctionCallData::getImpl()       noexcept { return pImpl.get(); }
const FunctionCallData::Impl* FunctionCallData::getImpl() const noexcept { return pImpl.get(); }

void FunctionCallData::reset() noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    pImpl = std::make_unique<Impl>();
}

uint64_t FunctionCallData::getNumHeapAllocations() const noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
    qiti::ScopedNoHeapAllocations noAlloc;
    
    auto impl = getImpl();
    assert(impl->numHeapAllocationsAfterFunctionCall >= impl->numHeapAllocationsBeforeFunctionCall);
    return impl->numHeapAllocationsAfterFunctionCall - impl->numHeapAllocationsBeforeFunctionCall;
}

uint64_t FunctionCallData::getAmountHeapAllocated() const noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
    qiti::ScopedNoHeapAllocations noAlloc;
    
    auto impl = getImpl();
    assert(impl->amountHeapAllocatedAfterFunctionCall >= impl->amountHeapAllocatedBeforeFunctionCall);
    return impl->amountHeapAllocatedAfterFunctionCall - impl->amountHeapAllocatedBeforeFunctionCall;
}

uint64_t QITI_API FunctionCallData::getTimeSpentInFunctionCpu_ms() const noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
    qiti::ScopedNoHeapAllocations noAlloc;
    
    if (getImpl()->timeSpentInFunctionNanosecondsCpu == 0)
        return 0; // prevent dividing by 0
    return getImpl()->timeSpentInFunctionNanosecondsCpu / 1000000;
}

uint64_t QITI_API FunctionCallData::getTimeSpentInFunctionCpu_ns() const noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->timeSpentInFunctionNanosecondsCpu;
}

uint64_t QITI_API FunctionCallData::getTimeSpentInFunctionWallClock_ms() const noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
    qiti::ScopedNoHeapAllocations noAlloc;
    
    if (getImpl()->timeSpentInFunctionNanosecondsWallClock == 0)
        return 0; // prevent dividing by 0
    return getImpl()->timeSpentInFunctionNanosecondsWallClock / 1000000;
}

uint64_t QITI_API FunctionCallData::getTimeSpentInFunctionWallClock_ns() const noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->timeSpentInFunctionNanosecondsWallClock;
}

std::thread::id QITI_API FunctionCallData::getThreadThatCalledFunction() const noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->callingThread;
}

const FunctionData* FunctionCallData::getCaller() const noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->caller;
}

bool FunctionCallData::didThrowException() const noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->numExceptionsThrown > 0;
}

uint64_t FunctionCallData::getNumExceptionsThrown() const noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->numExceptionsThrown;
}

} // namespace qiti
