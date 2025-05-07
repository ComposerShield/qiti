
#pragma once

#include "qiti_FunctionData.hpp"
#include "qiti_FunctionCallData.hpp"
#include "qiti_instrument.hpp"
#include "qiti_profile.hpp"
#include "qiti_utils.hpp"

//--------------------------------------------------------------------------

namespace qiti
{
/** */
void QITI_API resetAll() noexcept;

/** */
template <auto FuncPtr>
[[nodiscard]] const qiti::FunctionData* QITI_API getFunctionData() noexcept
{
    static constexpr auto* address = FuncPtr;
    return &getFunctionDataFromAddress(reinterpret_cast<void*>(address));
}

} // namespace qiti
