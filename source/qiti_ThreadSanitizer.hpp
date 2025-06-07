
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
#include <string>
#include <type_traits>

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
     Factory to create a detector for data races.
     
     @returns a ThreadSanitizer object that can be queried via passed() or failed() to
     determine if any data race occurred within the function/lambda called by run().
     
     Function pointer/lambdas are run in a forked process with thread sanitizer enabled.
     
     @see run()
     @see passed()
     @see failed()
     */
    [[nodiscard]] static std::unique_ptr<ThreadSanitizer> QITI_API createDataRaceDetector() noexcept;
    
    /**
     Factory to create a detector that checks if two functions are called in parallel.

     @returns a ThreadSanitizer object that can be queried via passed() or failed() to
     determine if the functions were ever called in parallel within the function/lambda
     called by run().
          
     @see run()
     @see passed()
     @see failed()
     */
    template<auto FuncPtr0, auto FuncPtr1>
    requires isFreeFunction<FuncPtr0>
    && isFreeFunction<FuncPtr1>
    [[nodiscard]] static std::unique_ptr<ThreadSanitizer> QITI_API createFunctionsCalledInParallelDetector() noexcept
    {
        static_assert(FuncPtr0 != FuncPtr1, "Functions must not be the same function.");
        return createFunctionsCalledInParallelDetector(FunctionData::getFunctionDataMutable<FuncPtr0>(),
                                                       FunctionData::getFunctionDataMutable<FuncPtr1>());
    }
    
    /**
     Factory to create a lock-order inversion detector.
    
     When calling run(), tracks every mutex-acquire; if two locks are
     ever taken in inverted order on different threads, it flags failure.
     
     @see run()
     @see passed()
     @see failed()
     
     TODO: Finish implementing
    */
    [[deprecated("WIP - Not finished implementing.")]]
    [[nodiscard]] static std::unique_ptr<ThreadSanitizer> QITI_API createPotentialDeadlockDetector() noexcept;
    
    /**
     @param func Function pointer or lambda that is immediately run and tested according to which ThreadSanitizer object you are using.
     
     Call passed()/failed() to query result.
     getReport() may also provide additional information.
     
     @see passed()
     @see failed()
     */
    virtual void QITI_API run(std::function<void()> func) noexcept = 0;
    
    /**
     Returns true if no errors were detected in the function(s)/lambda(s) called by run().
     
     @see run()
     */
    [[nodiscard]] bool QITI_API passed() noexcept;
    
    /** Convenience inverse of passed(). */
    [[nodiscard]] bool QITI_API failed() noexcept;
    
    /**
     Optional user-provided callback which is called immediately when a test fails.
     
     @see failed()
     */
    std::function<void()> onFail = nullptr;
    
    /**
     Returns a report if one is available. Not necessarily supported in every derived class.
     */
    [[nodiscard]] virtual std::string getReport(bool verbose) const noexcept;
    
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
    
    /** */
    void QITI_API_INTERNAL flagFailed() noexcept;
    
private:
    std::atomic<bool> _passed = true;
    
    /** Implementation. */
    static std::unique_ptr<ThreadSanitizer> QITI_API createFunctionsCalledInParallelDetector(FunctionData* func0,
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
