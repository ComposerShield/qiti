
#include "qiti_FunctionData.hpp"

#include "qiti_FunctionData_Impl.hpp"

#include <cassert>
#include <cstdint>
#include <dlfcn.h>

//--------------------------------------------------------------------------

namespace qiti
{

FunctionData::FunctionData(void* functionAddress) noexcept
{
    static_assert(sizeof(FunctionData::Impl)  <= FunctionData::ImplSize,  "Impl is too large for FunctionData::implStorage");
    static_assert(alignof(FunctionData::Impl) == FunctionData::ImplAlign, "Impl alignment stricter than FunctionData::implStorage");
    
    // Allocate Impl on the stack instead of the heap
    new (implStorage) Impl;
    
    auto* impl = getImpl();
    impl->address = functionAddress;
    
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

FunctionData::~FunctionData() noexcept
{
    getImpl()->~Impl();
}

FunctionData::Impl*       FunctionData::getImpl()       noexcept { return reinterpret_cast<Impl*>(implStorage); }
const FunctionData::Impl* FunctionData::getImpl() const noexcept { return reinterpret_cast<const Impl*>(implStorage); }

// Move constructor
FunctionData::FunctionData(FunctionData&& other) noexcept
{
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

const char* FunctionData::getFunctionName() const noexcept
{
    return getImpl()->functionNameReal.c_str();
}

qiti::uint FunctionData::getNumTimesCalled() const noexcept
{
    return getImpl()->numTimesCalled;
}

FunctionCallData FunctionData::getLastFunctionCall() const noexcept
{
    return getImpl()->lastCallData;
}

bool FunctionData::wasCalledOnThread(std::thread::id thread) const noexcept
{
    return getImpl()->threadsCalledOn.contains(thread);
}

} // namespace qiti
