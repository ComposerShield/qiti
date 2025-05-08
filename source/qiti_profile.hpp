
#pragma once

#include "qiti_API.hpp"

//--------------------------------------------------------------------------

namespace qiti
{
namespace profile
{
/** */
void QITI_API resetProfiling() noexcept;

/** Internal */
void QITI_API beginProfilingFunction(void* functionAddress) noexcept;

/** Internal */
void QITI_API endProfilingFunction(void* functionAddress) noexcept;

/** */
template<auto FuncPtr>
void QITI_API beginProfilingFunction() noexcept { beginProfilingFunction( reinterpret_cast<void*>(FuncPtr)); }

/** */
template <auto FuncPtr>
void QITI_API endProfilingFunction() noexcept { endProfilingFunction( reinterpret_cast<void*>(FuncPtr)); }

/** */
void QITI_API beginProfilingAllFunctions() noexcept;

/** */
void QITI_API endProfilingAllFunctions() noexcept;

/** */
[[nodiscard]] bool QITI_API isProfilingFunction(void* funcAddress) noexcept;

/** */
template<auto FuncPtr>
[[nodiscard]] bool QITI_API isProfilingFunction() noexcept
{
    return isProfilingFunction( reinterpret_cast<void*>(FuncPtr));
}

/** */
[[nodiscard]] unsigned long long QITI_API getNumHeapAllocationsOnCurrentThread() noexcept;

/** Internal */
void QITI_API_INTERNAL updateFunctionDataOnEnter(void* this_fn) noexcept;

/** Internal */
void QITI_API_INTERNAL updateFunctionDataOnExit(void* this_fn) noexcept;

} // namespace profile
} // namespace qiti
