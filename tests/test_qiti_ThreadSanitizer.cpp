
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

#include "qiti_ThreadSanitizer.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <regex>

//--------------------------------------------------------------------------

using namespace qiti::example::ThreadSanitizer;

//--------------------------------------------------------------------------

QITI_TEST_CASE("qiti::ThreadSanitizer::functionsNotCalledInParallel", ThreadSanitizerFunctionsNotCalledInParallel)
{
    qiti::ScopedQitiTest test;
    
    auto tsan = qiti::ThreadSanitizer::createFunctionsCalledInParallelDetector<testFunc0,
                                                                               testFunc1>();
    
    // Functions not called at all
    QITI_CHECK(tsan->passed());
    QITI_CHECK(! tsan->failed());
    
    // Functions called in sequence
    testFunc0();
    QITI_CHECK(tsan->passed());
    QITI_CHECK(! tsan->failed());
    
    testFunc1();
    QITI_CHECK(tsan->passed());
    QITI_CHECK(! tsan->failed());
        
    tsan->run([]
    {
        // Functions called in parallel
        std::thread t([]
        {
            for (auto i=0; i<100'000; ++i)
                testFunc0();
        });
        
        for (auto i=0; i<100'000; ++i)
            testFunc1(); // should race against thread t
        
        t.join();
    });
    
    QITI_CHECK(! tsan->passed());
    QITI_CHECK(tsan->failed());
    
    // Should have a report detailing the first infraction.
    auto report = tsan->getReport(false);
    QITI_REQUIRE(report != "");
    
    // Should have a report detailing each incident.
    auto verboseReport = tsan->getReport(true);
    QITI_REQUIRE(verboseReport != "");
}

#ifdef QITI_ENABLE_CLANG_THREAD_SANITIZER
QITI_TEST_CASE("qiti::ThreadSanitizer::createDataRaceDetector() does not produce false positive", ThreadSanitizerDataRaceDetectorNoFalsePositive)
{
    qiti::ScopedQitiTest test;
    
    auto noDataRace = [](){};
    auto dataRaceDetector = qiti::ThreadSanitizer::createDataRaceDetector();
    dataRaceDetector->run(noDataRace);
    QITI_REQUIRE(dataRaceDetector->passed());
    QITI_REQUIRE_FALSE(dataRaceDetector->failed());
    
    // Should have no report
    QITI_REQUIRE(dataRaceDetector->getReport(false) == "");
}

QITI_TEST_CASE("qiti::ThreadSanitizer::createDataRaceDetector() detects data race of global variable, ", ThreadSanitizerDataRaceDetectorGlobalVariable)
{
    qiti::ScopedQitiTest test;
    
    auto dataRace = []()
    {
        std::thread t(incrementCounter); // Intentional data race
        incrementCounter();              // Intentional data race
        t.join();
    };
    auto dataRaceDetector = qiti::ThreadSanitizer::createDataRaceDetector();
    dataRaceDetector->run(dataRace);
    QITI_REQUIRE(dataRaceDetector->failed());
    QITI_REQUIRE_FALSE(dataRaceDetector->passed());
    
    const auto* functionData = qiti::FunctionData::getFunctionData<incrementCounter>();
    const auto  functionName = functionData->getFunctionName();
    
    // Summary Report
    {
        /**
         Example report:
         "SUMMARY: ThreadSanitizer: data race qiti_example.cpp:107 in qiti::example::ThreadSanitizer::incrementCounter()"
         */
        auto report = dataRaceDetector->getReport(false);
        // Report is not empty
        QITI_REQUIRE(report != "");
        // Report contains function name that had the data race
        QITI_CHECK(report.find(functionName) != std::string::npos);
    }
    
    // Verbose Report
    {
        /**
         Example report:
         "==================\nWARNING: ThreadSanitizer: data race (pid=82969)\n"
         "Write of size 4 at 0x0001005cc000 by thread T5:\n"
         "#0 qiti::example::ThreadSanitizer::incrementCounter() qiti_example.cpp:107 "
         "(libqiti_example_target.0.0.1.dylib:arm64+0x39b0)\n"
         "    #1 decltype(std::declval<void (*)() noexcept>()()) "
         "std::__1::__invoke[abi:de180100]<void (*)() noexcept>(void (*&&)() noexcept) "
         "invoke.h:344 (qiti_tests:arm64+0x100055078)\n"
         etc.
         */
        auto verboseReport = dataRaceDetector->getReport(true);
        // Report is not empty
        QITI_REQUIRE(verboseReport != "");
        // Report contains function name that had the data race
        QITI_CHECK(verboseReport.find(functionName) != std::string::npos);
        // Report contains "write" since it was a write that produced the race
        QITI_CHECK(verboseReport.find("write") != std::string::npos);
    }
}

QITI_TEST_CASE("qiti::ThreadSanitizer::createDataRaceDetector() detects data race of member variable", ThreadSanitizerDataRaceDetectorMemberVariable)
{
    qiti::ScopedQitiTest test;
    
    auto dataRace = []()
    {
        TestClass testClass;
        std::atomic<int> ready{0};
        std::atomic<bool> go{false};
        std::thread a([&]{ testClass.incrementCounter(ready, go); }); // Intentional data race
        std::thread b([&]{ testClass.incrementCounter(ready, go); }); // Intentional data race
        while (ready.load(std::memory_order_relaxed) < 2) {}
        go.store(true, std::memory_order_release);
        a.join();
        b.join();
    };
    auto dataRaceDetector = qiti::ThreadSanitizer::createDataRaceDetector();
    dataRaceDetector->run(dataRace);
    QITI_REQUIRE(dataRaceDetector->failed());
    QITI_REQUIRE_FALSE(dataRaceDetector->passed());
}

#endif // QITI_ENABLE_CLANG_THREAD_SANITIZER

#if defined(__APPLE__) || defined(QITI_ENABLE_CLANG_THREAD_SANITIZER)

// Disable optimizations to prevent Release mode optimizations from interfering with intentional deadlock
#pragma clang optimize off

QITI_TEST_CASE("qiti::ThreadSanitizer::createPotentialDeadlockDetector() does not produce false positive", ThreadSanitizerDeadlockDetectorNoFalsePositive)
{
    qiti::ScopedQitiTest test;
    
    auto potentialDeadlockDetector = qiti::ThreadSanitizer::createPotentialDeadlockDetector();
    
    QITI_SECTION("Run code containing no locks.")
    {
        auto noMutexesAtAll = [](){};
        
        // Should pass
        potentialDeadlockDetector->run(noMutexesAtAll);
        QITI_REQUIRE(potentialDeadlockDetector->passed());
        QITI_REQUIRE_FALSE(potentialDeadlockDetector->failed());
    }
    
    QITI_SECTION("Run code containing 1 lock that does not deadlock.")
    {
        auto singleMutexWithNoDeadlock = []()
        {
            std::mutex mutex;
            
            // Mutex locked in parallel
            std::thread t([&mutex]()
            {
                for (auto i=0; i<1'000; ++i)
                {
                    std::scoped_lock<std::mutex> lock(mutex);
                    testFunc0();
                }
            });
            
            for (auto i=0; i<1'000; ++i)
            {
                std::scoped_lock<std::mutex> lock(mutex);
                testFunc1(); // should race against thread t
            }
            
            t.join();
        };
        
        // Should pass
        potentialDeadlockDetector->run(singleMutexWithNoDeadlock);
        QITI_REQUIRE(potentialDeadlockDetector->passed());
        QITI_REQUIRE_FALSE(potentialDeadlockDetector->failed());
    }
}

QITI_TEST_CASE("qiti::ThreadSanitizer::createPotentialDeadlockDetector() detects potential deadlock", ThreadSanitizerDeadlockDetectorDetectsDeadlock)
{
    qiti::ScopedQitiTest test;
    
    QITI_SECTION("Run code that inverts the order of mutex locking which implies a potential deadlock,"
                 "but does not actually deadlock here.")
    {
        auto potentialDeadlockDetector = qiti::ThreadSanitizer::createPotentialDeadlockDetector();
        
        auto createLockInversionPattern = []()
        {
            std::mutex mutexA;
            std::mutex mutexB;
            
            // First, establish A->B order in one thread
            std::thread t1([&]()
            {
                std::lock_guard<std::mutex> lockA(mutexA);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                std::lock_guard<std::mutex> lockB(mutexB);
                // TSan sees: A -> B
            });
            t1.join();
            
            // Then, establish B->A order in another thread
            // This creates the inversion pattern without concurrent access
            std::thread t2([&]()
            {
                std::lock_guard<std::mutex> lockB(mutexB);
                std::this_thread::sleep_for(std::chrono::milliseconds(1)); 
                std::lock_guard<std::mutex> lockA(mutexA);
                // TSan sees: B -> A (creates cycle A->B->A)
            });
            t2.join();
        };
        
        // Should fail
        potentialDeadlockDetector->run(createLockInversionPattern);
        QITI_REQUIRE_FALSE(potentialDeadlockDetector->passed());
        QITI_REQUIRE(potentialDeadlockDetector->failed());
    }
}

#pragma clang optimize on

#endif // defined(__APPLE__) || defined(QITI_ENABLE_CLANG_THREAD_SANITIZER)

//--------------------------------------------------------------------------
// New coverage tests
//--------------------------------------------------------------------------

QITI_TEST_CASE("qiti::ThreadSanitizer::rerun() re-executes cached function", ThreadSanitizerRerun)
{
    qiti::ScopedQitiTest test;

    auto tsan = qiti::ThreadSanitizer::createFunctionsCalledInParallelDetector<testFunc0,
                                                                               testFunc1>();

    // Run a function that calls both functions in parallel
    tsan->run([]
    {
        std::thread t([]
        {
            for (auto i = 0; i < 100'000; ++i)
                testFunc0();
        });

        for (auto i = 0; i < 100'000; ++i)
            testFunc1();

        t.join();
    });

    // Should have failed after run()
    QITI_REQUIRE(tsan->failed());

    // Now call rerun() which should re-execute the cached function
    tsan->rerun();

    // Should still be failed after rerun()
    QITI_REQUIRE(tsan->failed());

    // Report should still be available
    auto report = tsan->getReport(false);
    QITI_CHECK(report != "");
}

QITI_TEST_CASE("qiti::ThreadSanitizer::rerun() does nothing when run() was never called", ThreadSanitizerRerunWithoutRun)
{
    qiti::ScopedQitiTest test;

    auto tsan = qiti::ThreadSanitizer::createFunctionsCalledInParallelDetector<testFunc0,
                                                                               testFunc1>();

    // Should pass initially
    QITI_CHECK(tsan->passed());

    // rerun() should do nothing since run() was never called
    tsan->rerun();

    // Should still pass
    QITI_CHECK(tsan->passed());
}

QITI_TEST_CASE("qiti::ThreadSanitizer::onFail callback is invoked on failure", ThreadSanitizerOnFailCallback)
{
    qiti::ScopedQitiTest test;

    auto tsan = qiti::ThreadSanitizer::createFunctionsCalledInParallelDetector<testFunc0,
                                                                               testFunc1>();

    // Set up onFail callback
    std::atomic<int> failCallbackCount{0};
    tsan->onFail = [&failCallbackCount]()
    {
        failCallbackCount.fetch_add(1, std::memory_order_relaxed);
    };

    // Run functions in parallel to trigger failure
    tsan->run([]
    {
        std::thread t([]
        {
            for (auto i = 0; i < 100'000; ++i)
                testFunc0();
        });

        for (auto i = 0; i < 100'000; ++i)
            testFunc1();

        t.join();
    });

    // Detector should have failed
    QITI_REQUIRE(tsan->failed());

    // onFail callback should have been called at least once
    QITI_CHECK(failCallbackCount.load(std::memory_order_relaxed) > 0);
}

#if defined(__APPLE__)

// Disable optimizations to prevent Release mode optimizations from interfering with lock ordering
#pragma clang optimize off

QITI_TEST_CASE("qiti::ThreadSanitizer::LockOrderInversionDetector out-of-order release", ThreadSanitizerOutOfOrderRelease)
{
    qiti::ScopedQitiTest test;

    auto potentialDeadlockDetector = qiti::ThreadSanitizer::createPotentialDeadlockDetector();

    // Acquire locks A then B, but release A before B (out-of-order release).
    // This is not a lock inversion (the acquire order is consistent), but it
    // triggers the out-of-order release path in LockOrderInversionDetector::onRelease().
    auto outOfOrderRelease = []()
    {
        std::mutex mutexA;
        std::mutex mutexB;

        mutexA.lock();
        mutexB.lock();

        // Release A first (out-of-order: should release B first since it was acquired last)
        mutexA.unlock();
        mutexB.unlock();
    };

    potentialDeadlockDetector->run(outOfOrderRelease);

    // The out-of-order release should flag failure
    QITI_REQUIRE(potentialDeadlockDetector->failed());
}

#pragma clang optimize on

#endif // defined(__APPLE__)
