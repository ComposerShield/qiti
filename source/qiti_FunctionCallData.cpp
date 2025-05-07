
#include "qiti_FunctionData.hpp"

#include "qiti_FunctionCallData_Impl.hpp"

#include <cassert>
#include <chrono>
#include <cstdint>

//--------------------------------------------------------------------------

namespace qiti
{
FunctionCallData::FunctionCallData() noexcept
{
    static_assert(sizeof(FunctionCallData::Impl)  <= FunctionCallData::ImplSize,  "Impl is too large for FunctionData::implStorage");
    static_assert(alignof(FunctionCallData::Impl) == FunctionCallData::ImplAlign, "Impl alignment stricter than FunctionData::implStorage");
    
    // Allocate Impl on the stack instead of the heap
    new (implStorage) Impl;
}

FunctionCallData::~FunctionCallData() noexcept
{
    getImpl()->~Impl();
}

// Move constructor
FunctionCallData::FunctionCallData(FunctionCallData&& other) noexcept
{
    // move‐construct into our storage
    new (implStorage) Impl(std::move(*other.getImpl()));
    // destroy their Impl so we can re‐init it
    other.getImpl()->~Impl();
    // default‐construct theirs back into a valid (empty) state
    new (other.implStorage) Impl();
}

// Move assignment
FunctionCallData& FunctionCallData::operator=(FunctionCallData&& other) noexcept
{
    if (this != &other) {
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

// Copy constructor
FunctionCallData::FunctionCallData(const FunctionCallData& other) noexcept
{
    // placement‐new a copy of their Impl into our storage
    new (implStorage) Impl(*other.getImpl());
}

// Copy assignment
FunctionCallData FunctionCallData::operator=(const FunctionCallData& other) noexcept
{
    if (this != &other)
    {
        // destroy our current Impl
        getImpl()->~Impl();
        // placement‐new a copy of theirs
        new (implStorage) Impl(*other.getImpl());
    }
    return *this;
}

FunctionCallData::Impl*       FunctionCallData::getImpl()       noexcept { return reinterpret_cast<Impl*>(implStorage); }
const FunctionCallData::Impl* FunctionCallData::getImpl() const noexcept { return reinterpret_cast<const Impl*>(implStorage); }

void FunctionCallData::reset() noexcept
{
    getImpl()->~Impl();
    new (implStorage) Impl; // re-initialize
}

uint FunctionCallData::getNumHeapAllocations() const noexcept
{
    auto impl = getImpl();
    assert(impl->numHeapAllocationsAfterFunctionCall >= impl->numHeapAllocationsBeforeFunctionCall);
    return impl->numHeapAllocationsAfterFunctionCall - impl->numHeapAllocationsBeforeFunctionCall;
}

} // namespace qiti
