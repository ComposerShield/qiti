
#pragma once

#include "qiti_TypeData.hpp"

//--------------------------------------------------------------------------

namespace qiti
{
enum class FunctionType
{
    regular,
    constructor,
    destructor
};

struct TypeData::Impl
{
public:
    QITI_API_INTERNAL Impl() = default;
    QITI_API_INTERNAL ~Impl() = default;
    
    char name[128];
    
    unsigned long long numTimesConstructed = 0;
    unsigned long long numTimesDestructed  = 0;
};
} // namespace qiti
