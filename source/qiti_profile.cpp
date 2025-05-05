
#include "qiti_profile.hpp"

#include "qiti_FunctionCallData_Impl.hpp"
#include "qiti_FunctionCallData.hpp"
#include "qiti_FunctionData_Impl.hpp"
#include "qiti_FunctionData.hpp"

#include <mutex>
#include <unordered_set>

//--------------------------------------------------------------------------

std::unordered_set<void*> g_functionsToProfile;
bool g_profileAllFunctions = false;

struct Init_g_functionsToProfile
{
    Init_g_functionsToProfile()
    {
        g_functionsToProfile.reserve(256);
    }
};
static const Init_g_functionsToProfile init_g_functionsToProfile;

inline static thread_local uint64_t g_numHeapAllocationsOnCurrentThread = 0;

extern thread_local std::function<void()> g_onNextHeapAllocation;
extern std::recursive_mutex qiti_global_lock;

//--------------------------------------------------------------------------

void* operator new(size_t size)
{
    ++g_numHeapAllocationsOnCurrentThread;
    if (auto callback = std::exchange(g_onNextHeapAllocation, nullptr))
        callback();
    
    // Original implementation
    void* p = malloc(size);
    return p;
}

//--------------------------------------------------------------------------

namespace qiti
{
namespace profile
{
void resetProfiling() noexcept
{
    // Prevent any qiti work while we disable profiling
    std::scoped_lock<std::recursive_mutex> lock(qiti_global_lock);
    
    g_functionsToProfile.clear();
    g_profileAllFunctions = false;
    g_numHeapAllocationsOnCurrentThread = 0;
}

void beginProfilingFunction(void* functionAddress) noexcept
{
    g_functionsToProfile.insert(functionAddress);
    
    // This adds the function to our function map
    (void)qiti::getFunctionDataFromAddress(functionAddress);
}

void endProfilingFunction(void* functionAddress) noexcept
{
    g_functionsToProfile.erase(functionAddress);
}

void beginProfilingAllFunctions() noexcept
{
    g_profileAllFunctions = true;
}

void endProfilingAllFunctions() noexcept
{
    g_profileAllFunctions = false;
}

bool shouldProfileFunction(void* funcAddress) noexcept
{
    return g_profileAllFunctions || g_functionsToProfile.contains(funcAddress);
}

void updateFunctionDataOnEnter(void* this_fn) noexcept
{
    auto& functionData = qiti::getFunctionDataFromAddress(this_fn);
    auto* impl = functionData.getImpl();
    std::thread::id thisThread = std::this_thread::get_id();
    
    ++impl->numTimesCalled;
    impl->threadsCalledOn.insert(thisThread);
    
    impl->lastCallData.reset(); // Deletes previous impl
    
    auto* lastCallImpl = impl->lastCallData.getImpl();
    lastCallImpl->begin_time = std::chrono::steady_clock::now();
    lastCallImpl->callingThread = std::this_thread::get_id();
#ifndef QITI_DISABLE_HEAP_ALLOCATION_TRACKER
    impl->lastCallData.getImpl()->numHeapAllocationsBeforeFunctionCall = g_numHeapAllocationsOnCurrentThread;
#endif
}

void updateFunctionDataOnExit(void* this_fn) noexcept
{
    auto& functionData = qiti::getFunctionDataFromAddress(this_fn);
    auto* impl = functionData.getImpl();
    auto* callImpl = impl->lastCallData.getImpl();
    
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - callImpl->begin_time);

    callImpl->end_time = end_time;
    callImpl->timeSpentInFunctionNanoseconds = static_cast<qiti::uint>(elapsed_ns.count());
#ifndef QITI_DISABLE_HEAP_ALLOCATION_TRACKER
    callImpl->numHeapAllocationsAfterFunctionCall = g_numHeapAllocationsOnCurrentThread;
#endif
}
} // namespace profile
} // namespace qiti
