
// Copyright (c) 2025 Adam Shield
// SPDX-License-Identifier: MIT

#pragma once

#include "qiti_FunctionData.hpp"

#include <cstdint>
#include <string>
#include <thread>
#include <unordered_set>

//--------------------------------------------------------------------------

namespace qiti
{
enum class FunctionType
{
    regular,
    constructor,
    destructor
};

struct FunctionData::Impl
{
public:
    QITI_API_INTERNAL Impl() = default;
    QITI_API_INTERNAL ~Impl() = default;
    
    char functionNameMangled[128];
    char functionNameReal[128];
    void* address = nullptr;
    
    uint numTimesCalled = 0;
    uint averageTimeSpentInFunctionNanoseconds = 0;
    std::unordered_set<std::thread::id> threadsCalledOn{};
    
    FunctionType functionType = FunctionType::regular;
    
    FunctionCallData lastCallData{};
};
} // namespace qiti
