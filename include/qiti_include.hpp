
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
void QITI_API shutdown();

/** */
template <auto FuncPtr>
[[nodiscard]] const qiti::FunctionData* QITI_API getFunctionData() { return getFunctionDataImpl<FuncPtr>(); }

} // namespace qiti
