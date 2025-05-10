
#pragma once

#include "qiti_API.hpp"

#include <type_traits>
#include <typeindex>
#include <typeinfo>

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
requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
void QITI_API inline beginProfilingFunction() noexcept { beginProfilingFunction( reinterpret_cast<void*>(FuncPtr)); }

/** */
template <auto FuncPtr>
requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
void QITI_API inline endProfilingFunction() noexcept { endProfilingFunction( reinterpret_cast<void*>(FuncPtr)); }

/** */
void QITI_API beginProfilingAllFunctions() noexcept;

/** */
void QITI_API endProfilingAllFunctions() noexcept;

/** */
[[nodiscard]] bool QITI_API isProfilingFunction(void* funcAddress) noexcept;

/** */
template<auto FuncPtr>
requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
[[nodiscard]] inline bool QITI_API isProfilingFunction() noexcept
{
    return isProfilingFunction( reinterpret_cast<void*>(FuncPtr));
}

/** Internal */
void QITI_API beginProfilingType(std::type_index functionAddress) noexcept;

/** Internal */
void QITI_API endProfilingType(std::type_index functionAddress) noexcept;

/** */
template<typename Type>
inline void QITI_API beginProfilingType() noexcept { beginProfilingType( typeid(Type) ); }

/** */
template <typename Type>
inline void QITI_API endProfilingType() noexcept { endProfilingType( typeid(Type) ); }

/** */
[[nodiscard]] unsigned long long QITI_API getNumHeapAllocationsOnCurrentThread() noexcept;

/** Internal */
void QITI_API_INTERNAL updateFunctionDataOnEnter(void* this_fn) noexcept;

/** Internal */
void QITI_API_INTERNAL updateFunctionDataOnExit(void* this_fn) noexcept;

} // namespace profile
} // namespace qiti
