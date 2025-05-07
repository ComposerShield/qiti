
#pragma once

#include "qiti_FunctionCallData.hpp"

#include <thread>

//--------------------------------------------------------------------------

namespace qiti
{
/**
 Abtracts a function and its history of use
 */
class FunctionData
{
public:
    /** */
    [[nodiscard]] const char* QITI_API getFunctionName() const noexcept;
    
    /** */
    [[nodiscard]] uint QITI_API getNumTimesCalled() const noexcept;
    
    /**
     Returns true if function was called on the provided thread.
     
     Call example:
     
     */
    [[nodiscard]] bool QITI_API wasCalledOnThread(std::thread::id thread) const noexcept;

    /** */
    [[nodiscard]] FunctionCallData QITI_API getLastFunctionCall() const noexcept;
    
    /** Internal */
    QITI_API_INTERNAL FunctionData(void* functionAddress) noexcept;
    /** Internal */
    QITI_API_INTERNAL ~FunctionData() noexcept;
    
    struct Impl;
    /** Internal */
    [[nodiscard]] Impl* QITI_API_INTERNAL getImpl() noexcept;
    /** Internal */
    [[nodiscard]] const Impl* QITI_API_INTERNAL getImpl() const noexcept;
    
    /** Internal Move Constructor */
    QITI_API_INTERNAL FunctionData(FunctionData&& other) noexcept;
    /** Internal Move Operator */
    [[nodiscard]] FunctionData& QITI_API_INTERNAL operator=(FunctionData&& other) noexcept;
    
private:
    // Stack-based pimpl idiom
    static constexpr std::size_t ImplSize  = 128;
    static constexpr std::size_t ImplAlign =  8;
    alignas(ImplAlign) unsigned char implStorage[ImplSize];
    
    FunctionData(const FunctionData&) = delete;
    FunctionData& operator=(const FunctionData&) = delete;
};
} // namespace qiti
