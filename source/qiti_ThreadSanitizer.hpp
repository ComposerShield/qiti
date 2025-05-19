
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

namespace qiti
{
/**
 */
class ThreadSanitizer
{
public:
    /** Runs the function/lambda and reports if data races were detected. */
    [[nodiscard]] static std::unique_ptr<ThreadSanitizer> QITI_API createDataRaceDetector(std::function<void()>) noexcept;
    
    /** */
    template<auto FuncPtr0, auto FuncPtr1>
    requires std::is_function_v<std::remove_pointer_t<decltype(FuncPtr0)>>
    && std::is_function_v<std::remove_pointer_t<decltype(FuncPtr1)>>
    [[nodiscard]] static ThreadSanitizer QITI_API functionsNotCalledInParallel() noexcept
    {
        return functionsNotCalledInParallel(reinterpret_cast<void*>(FuncPtr0),
                                            reinterpret_cast<void*>(FuncPtr1));
    }
    
    /** */
    [[nodiscard]] virtual bool QITI_API passed() noexcept;
    
    /** */
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
    /** */
    static ThreadSanitizer QITI_API functionsNotCalledInParallel(void* func0, void* func1) noexcept;
    
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
