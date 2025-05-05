
#pragma once

#include "qiti_utils.hpp"

//--------------------------------------------------------------------------

namespace qiti
{
/**
 Abtracts a specific call of a specific function
 */
class FunctionCallData
{
public:
    /** */
    [[nodiscard]] uint QITI_API getNumHeapAllocations() const noexcept;
    
    //--------------------------------------------------------------------------
    
    /** Internal */
    QITI_API_INTERNAL FunctionCallData() noexcept;
    /** */
    QITI_API ~FunctionCallData() noexcept;
    
    struct Impl;
    /** Internal */
    [[nodiscard]] Impl* QITI_API_INTERNAL getImpl() const noexcept;
    
    /** Internal */
    void QITI_API_INTERNAL reset() noexcept;
    
    /** Internal Move Constructor */
    QITI_API_INTERNAL FunctionCallData(FunctionCallData&& other) noexcept;
    /** Internal Move Operator */
    [[nodiscard]] FunctionCallData& QITI_API_INTERNAL operator=(FunctionCallData&& other) noexcept;
    /** Internal Copy Constructor */
    FunctionCallData(const FunctionCallData&) noexcept;
    /** Internal Move Operator */
    [[nodiscard]] FunctionCallData operator=(const FunctionCallData&) noexcept;
    
private:
    Impl* impl;
};
} // namespace qiti
