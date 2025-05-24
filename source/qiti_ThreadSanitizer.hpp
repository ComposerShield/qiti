
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_ThreadSanitizer.hpp
 *
 * @author   Adam Shield
 * @date     2025-05-16
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#pragma once

#include "qiti_API.hpp"

#include "qiti_FunctionData.hpp"

#include <functional>
#include <memory>
#include <type_traits>

//--------------------------------------------------------------------------

/**
 \internal
 Settings for Clang ThreadSanitizer
 */
extern "C" __attribute__((visibility("default")))
const char* QITI_API __tsan_default_options();

//--------------------------------------------------------------------------
namespace qiti
{
//--------------------------------------------------------------------------
/**
 Base class for ThreadSanitizer checks.

 Provides a standardized API for generating different thread safety checks (e.g. running functions in a
 forked process with ThreadSanitizer enabled) and querying pass/fail status.
 */
class ThreadSanitizer
{
public:
    /**
     Factory to create a detector for data race checking.
     
     @param func Function pointer or lambda that is immediately run in a forked process with ThreadSanitizer enabled.
     
     @returns a ThreadSanitizer object that can be queried via passed() or failed() to
     determine if a data race occured within the funtion/lambda.
     */
    [[nodiscard]] static std::unique_ptr<ThreadSanitizer> QITI_API createDataRaceDetector(std::function<void()> func) noexcept;
    
    /**
     Factory to create a detector that checks if two functions are called in parallel.

     @returns a ThreadSanitizer object that can be queried via passed() or failed() to
     determine if the functions were called in parallel.
     */
    template<auto FuncPtr0, auto FuncPtr1>
    requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr0)>>
    && std::is_function_v<std::remove_pointer_t<decltype(FuncPtr1)>>
    [[nodiscard]] static std::unique_ptr<ThreadSanitizer> QITI_API functionsNotCalledInParallel() noexcept
    {
        return functionsNotCalledInParallel(FunctionData::getFunctionDataMutable<FuncPtr0>(),
                                            FunctionData::getFunctionDataMutable<FuncPtr1>());
    }
    
    /** Returns true if no errors were detected. */
    [[nodiscard]] virtual bool QITI_API passed() noexcept;
    
    /** Convenience inverse of passed(). */
    [[nodiscard]] bool QITI_API failed() noexcept;
    
    //--------------------------------------------------------------------------
    // Doxygen - Begin Internal Documentation
    /** \cond INTERNAL */
    //--------------------------------------------------------------------------
    
    /** */
    virtual QITI_API ~ThreadSanitizer() noexcept;

    /** Move Constructor */
    QITI_API ThreadSanitizer(ThreadSanitizer&& other) noexcept;
    /** Move Assignment */
    [[nodiscard]] ThreadSanitizer& QITI_API operator=(ThreadSanitizer&& other) noexcept;
    
protected:    
    /** */
    QITI_API_INTERNAL ThreadSanitizer() noexcept;
    
private:
    /** Implementation */
    static std::unique_ptr<ThreadSanitizer> QITI_API functionsNotCalledInParallel(FunctionData* func0,
                                                                                  FunctionData* func1) noexcept;
    
    /** Copy Constructor (deleted) */
    ThreadSanitizer(const ThreadSanitizer&) = delete;
    /** Copy Assignment (deleted) */
    ThreadSanitizer& operator=(const ThreadSanitizer&) = delete;
    
    //--------------------------------------------------------------------------
    /** \endcond */
    // Doxygen - End Internal Documentation
    //--------------------------------------------------------------------------
};
} // namespace qiti
