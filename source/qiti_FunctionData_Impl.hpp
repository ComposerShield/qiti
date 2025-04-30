
#pragma once

#include "qiti_FunctionData.hpp"

#include <cstdint>
#include <string>

//--------------------------------------------------------------------------

namespace qiti
{
struct FunctionData::Impl
{
public:
    QITI_API_INTERNAL Impl() = default;
    QITI_API_INTERNAL ~Impl() = default;
    
    std::string functionNameMangled;
    std::string functionNameReal;
    int64_t numTimesCalled = 0;
    int64_t averageTimeSpentInFunctionNanoseconds = 0;
    
    FunctionCallData lastCallData;
};
} // namespace qiti
