
#include "qiti_FunctionData.hpp"

#include "qiti_FunctionCallData_Impl.hpp"

#include <cassert>
#include <chrono>
#include <cstdint>

//--------------------------------------------------------------------------

namespace qiti
{
FunctionCallData::FunctionCallData()
{
    impl = new Impl;
}

FunctionCallData::~FunctionCallData()
{
    delete impl;
}

FunctionCallData::FunctionCallData(FunctionCallData&& other)
{
    impl = other.impl;
    other.impl = nullptr;
}

FunctionCallData& FunctionCallData::operator=(FunctionCallData&& other) noexcept
{
    impl = other.impl;
    other.impl = nullptr;
    return *this;
}

FunctionCallData::FunctionCallData(const FunctionCallData& other)
{
    impl = new Impl(*other.impl);
}

FunctionCallData FunctionCallData::operator=(const FunctionCallData& other)
{
    FunctionCallData copy;
    copy.impl = new Impl(*other.impl);
    return copy;
}

FunctionCallData::Impl* FunctionCallData::getImpl() const noexcept
{
    return impl;
}

void FunctionCallData::reset() noexcept
{
    delete impl;
    impl = new Impl;
}

uint FunctionCallData::getNumHeapAllocations() const noexcept
{
    assert(impl->numHeapAllocationsAfterFunctionCall >= impl->numHeapAllocationsBeforeFunctionCall);
    return impl->numHeapAllocationsAfterFunctionCall - impl->numHeapAllocationsBeforeFunctionCall;
}

} // namespace qiti
