
#pragma once

#include "qiti_FunctionData.hpp"
#include "qiti_FunctionCallData.hpp"
#include "qiti_instrument.hpp"
#include "qiti_profile.hpp"
#include "qiti_utils.hpp"

#include <type_traits>

//--------------------------------------------------------------------------

namespace qiti
{
/** */
void QITI_API resetAll() noexcept;

/** */
template <auto FuncPtr>
requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr)>>
[[nodiscard]] const qiti::FunctionData* QITI_API getFunctionData() noexcept
{
    static constexpr auto* address = FuncPtr;
    return &getFunctionDataFromAddress(reinterpret_cast<void*>(address));
}

} // namespace qiti
