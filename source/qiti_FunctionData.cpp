
#include "qiti_FunctionData.hpp"

#include "qiti_FunctionData_Impl.hpp"

#include <cassert>
#include <cstdint>
#include <dlfcn.h>

//--------------------------------------------------------------------------

namespace qiti
{

FunctionData::FunctionData(void* functionAddress)
{
    impl = new Impl;
    
    Dl_info info;
    if (dladdr(functionAddress, &info))
    {
        impl->functionNameMangled = info.dli_sname;
        char functionNameDemangled[256];
        qiti::demangle(info.dli_sname, functionNameDemangled, sizeof(functionNameDemangled));
        impl->functionNameReal = functionNameDemangled;
    }
    else
    {
        std::terminate(); // TODO: will this ever happen?
    }
}

FunctionData::~FunctionData()
{
    delete impl;
}

FunctionData::FunctionData(FunctionData&& other)
{
    impl = other.impl;
    other.impl = nullptr;
}

FunctionData& FunctionData::operator=(FunctionData&& other) noexcept
{
    assert(false);
}

FunctionData::Impl* FunctionData::getImpl() const noexcept
{
    return impl;
}

const char* FunctionData::getFunctionName() const noexcept
{
    return impl->functionNameReal.c_str();
}

qiti::uint FunctionData::getNumTimesCalled() const noexcept
{
    return impl->numTimesCalled;
}

FunctionCallData FunctionData::getLastFunctionCall() const noexcept
{
    return impl->lastCallData;
}

} // namespace qiti
