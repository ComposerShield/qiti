
#pragma once

#include "qiti_FunctionCallData.hpp"

#include <chrono>
#include <cstdint>

namespace qiti
{
struct FunctionCallData::Impl
{
    std::chrono::steady_clock::time_point begin_time;
    std::chrono::steady_clock::time_point end_time;
    int64_t timeSpentInFunctionNanoseconds = 0;
    
    int64_t numHeapAllocationsBeforeFunctionCall = 0;
    int64_t numHeapAllocationsAfterFunctionCall  = 0;
};
} // namespace qiti
