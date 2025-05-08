
#include "qiti_FunctionData.hpp"

#include "qiti_FunctionData_Impl.hpp"

#include "qiti_ScopedNoHeapAllocations.hpp"

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
    
    qiti::ScopedNoHeapAllocations noAlloc;
    
    // Allocate Impl on the stack instead of the heap
    new (implStorage) Impl;
        
    auto* impl = getImpl();
    
    impl->address = functionAddress;
    
    Dl_info info;
    if (dladdr(functionAddress, &info))
    {
        strncpy(impl->functionNameMangled, info.dli_sname, sizeof(impl->functionNameMangled) - 1);
        char functionNameDemangled[256];
        qiti::demangle(info.dli_sname, functionNameDemangled, sizeof(functionNameDemangled));
        strncpy(impl->functionNameReal, functionNameDemangled, sizeof(impl->functionNameReal) - 1);
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
    qiti::ScopedNoHeapAllocations noAlloc;
    
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
    qiti::ScopedNoHeapAllocations noAlloc;
    
    if (this != &other)
    {
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
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->functionNameReal;
}

const char* FunctionData::getMangledFunctionName() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->functionNameMangled;
}

qiti::uint FunctionData::getNumTimesCalled() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->numTimesCalled;
}

FunctionCallData FunctionData::getLastFunctionCall() const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->lastCallData;
}

bool FunctionData::wasCalledOnThread(std::thread::id thread) const noexcept
{
    qiti::ScopedNoHeapAllocations noAlloc;
    
    return getImpl()->threadsCalledOn.contains(thread);
}

} // namespace qiti
