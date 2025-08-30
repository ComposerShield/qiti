
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
 
 Use the factory functions to create specific types of ThreadSanitizer detectors:
 - createDataRaceDetector() - Detects data races in your code
 - createFunctionsCalledInParallelDetector<&func1, &func2>() - Checks if two functions are called in parallel
 - createPotentialDeadlockDetector() - Detects lock-order inversions
 
 @code
 auto detector = ThreadSanitizer::createDataRaceDetector();
 detector->run([]() {
     // Your potentially racy code here
 });
 if (detector->failed()) {
     // Handle race condition
 }
 @endcode
 */
class ThreadSanitizer
{
public:
    /**
     Factory to create a detector for data races.
     
     @returns a ThreadSanitizer object that can be queried via passed() or failed() to
     determine if any data race occurred within the function/lambda called by run().
     
     Function pointer/lambdas are run in a forked process with thread sanitizer enabled.
     
     @code
     auto detector = ThreadSanitizer::createDataRaceDetector();
     detector->run([]() {
         // Code that might have data races
         std::thread t1([]() { sharedVariable++; });
         std::thread t2([]() { sharedVariable++; });
         t1.join(); t2.join();
     });
     REQUIRE(detector->failed()); // Should detect race
     @endcode
     
     @see run()
     @see passed()
     @see failed()
     */
#ifdef QITI_ENABLE_CLANG_THREAD_SANITIZER // set by CMake with -DQITI_ENABLE_CLANG_THREAD_SANITIZER=ON
    [[nodiscard]] QITI_API static std::unique_ptr<ThreadSanitizer> createDataRaceDetector() noexcept;
#endif // QITI_ENABLE_CLANG_THREAD_SANITIZER
    
    /**
     Factory to create a detector that checks if two functions are called in parallel.

     @returns a ThreadSanitizer object that can be queried via passed() or failed() to
     determine if the functions were ever called in parallel within the function/lambda
     called by run().
     
     @code
     auto detector = ThreadSanitizer::createFunctionsCalledInParallelDetector<&func1, &func2>();
     detector->run([]() {
         std::thread t1([]() { func1(); });
         std::thread t2([]() { func2(); });
         t1.join(); t2.join();
     });
     REQUIRE(detector->passed()); // Should detect parallel execution
     @endcode
          
     @see run()
     @see passed()
     @see failed()
     */
    template<auto FuncPtr0, auto FuncPtr1>
    requires isFreeFunction<FuncPtr0>
    && isFreeFunction<FuncPtr1>
    [[nodiscard]] QITI_API_INLINE static std::unique_ptr<ThreadSanitizer> createFunctionsCalledInParallelDetector() noexcept
    {
        static_assert(FuncPtr0 != FuncPtr1, "Functions must not be the same function.");
        return createFunctionsCalledInParallelDetector(FunctionData::getFunctionDataMutable<FuncPtr0>(),
                                                       FunctionData::getFunctionDataMutable<FuncPtr1>());
    }
    
    /**
     Factory to create a lock-order inversion detector.
     
     Available on:
     - macOS: Always available, uses custom lock-order tracking
     - Linux: Requires QITI_ENABLE_CLANG_THREAD_SANITIZER, uses TSan's deadlock detection
     - Windows: Not supported
     
     When calling run(), tracks every mutex-acquire; if two locks are
     ever taken in inverted order on different threads, it flags failure.
     
     @code
     std::mutex mutex1, mutex2;
     auto detector = ThreadSanitizer::createPotentialDeadlockDetector();
     detector->run([]() {
         std::thread t1([&]() {
             std::lock_guard lock1(mutex1);
             std::lock_guard lock2(mutex2); // Order: mutex1 -> mutex2
         });
         std::thread t2([&]() {
             std::lock_guard lock2(mutex2);
             std::lock_guard lock1(mutex1); // Order: mutex2 -> mutex1 (inversion!)
         });
         t1.join(); t2.join();
     });
     REQUIRE(detector->failed()); // Should detect potential deadlock
     @endcode
     
     @see run()
     @see passed()
     @see failed()
    */
#if defined(__APPLE__) || defined(QITI_ENABLE_CLANG_THREAD_SANITIZER)
    [[nodiscard]] QITI_API static std::unique_ptr<ThreadSanitizer> createPotentialDeadlockDetector() noexcept;
#endif
    
    /**
     @param func Function pointer or lambda that is immediately run and tested according to which ThreadSanitizer object you are using.
     
     Call passed()/failed() to query result.
     getReport() may also provide additional information.
     
     @see passed()
     @see failed()
     */
    QITI_API virtual void run(std::function<void()> func) noexcept = 0;
    
    /**
     Re-run the last function that was passed to run().
     
     This method runs the cached function from the previous call to run().
     If run() has never been called, this method does nothing.
     
     @see run()
     */
    QITI_API void rerun() noexcept;
    
    /**
     Returns true if no errors were detected in the function(s)/lambda(s) called by run().
     
     @see run()
     */
    [[nodiscard]] QITI_API bool passed() noexcept;
    
    /** Convenience inverse of passed(). */
    [[nodiscard]] QITI_API bool failed() noexcept;
    
    /**
     Optional user-provided callback which is called immediately when a test fails.
     
     @see failed()
     */
    std::function<void()> onFail = nullptr;
    
    /**
     Returns a report if one is available. Not necessarily supported in every derived class.
     */
    [[nodiscard]] QITI_API virtual std::string getReport(bool verbose) const noexcept;
    
    //--------------------------------------------------------------------------
    // Doxygen - Begin Internal Documentation
    /** \cond INTERNAL */
    //--------------------------------------------------------------------------
    
    /** */
    QITI_API virtual ~ThreadSanitizer() noexcept;

    /** Move Constructor */
    QITI_API ThreadSanitizer(ThreadSanitizer&& other) noexcept;
    /** Move Assignment */
    [[nodiscard]] QITI_API ThreadSanitizer& operator=(ThreadSanitizer&& other) noexcept;
    
protected:    
    /** */
    QITI_API_INTERNAL ThreadSanitizer() noexcept;
    
    /** */
    QITI_API_INTERNAL void flagFailed() noexcept;
    
protected:
    std::atomic<bool> _passed = true;
    std::function<void()> _cachedFunction = nullptr;
    
private:
    
    /** Implementation. */
    QITI_API static std::unique_ptr<ThreadSanitizer> createFunctionsCalledInParallelDetector(FunctionData* func0,
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

