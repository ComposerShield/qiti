
#pragma once

#include "qiti_utils.hpp"

//--------------------------------------------------------------------------

namespace qiti
{
namespace profile
{
/** */
void QITI_API resetProfiling() noexcept;

/** */
void QITI_API beginProfilingFunction(void* functionAddress) noexcept;

/** */
void QITI_API endProfilingFunction(void* functionAddress) noexcept;

/** */
template<auto FuncPtr>
void QITI_API beginProfilingFunction() noexcept { beginProfilingFunction( reinterpret_cast<void*>(FuncPtr)); }

/** */
template <auto FuncPtr>
void QITI_API endProfilingFunction() noexcept { endProfilingFunction(FuncPtr); }

/** */
void QITI_API beginProfilingAllFunctions() noexcept;

/** */
void QITI_API endProfilingAllFunctions() noexcept;

/** Internal */
[[nodiscard]] bool QITI_API_INTERNAL shouldProfileFunction(void* funcAddress) noexcept;
    
} // namespace profile
} // namespace qiti
