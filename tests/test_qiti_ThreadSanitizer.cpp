
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"
// Basic Catch2 macros
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <regex>

//--------------------------------------------------------------------------

using namespace qiti::example::ThreadSanitizer;

//--------------------------------------------------------------------------

TEST_CASE("qiti::ThreadSanitizer::functionsNotCalledInParallel")
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

TEST_CASE("qiti::ThreadSanitizer::createDataRaceDetector() does not produce false positive")
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

TEST_CASE("qiti::ThreadSanitizer::createDataRaceDetector() detects data race of global variable, "
          "qiti::ThreadSanitizer::getReport()")
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

#if defined(__APPLE__) // Turning off this test in Linux because it is just too brittle in CI
TEST_CASE("qiti::ThreadSanitizer::createDataRaceDetector() detects data race of member variable")
{
    qiti::ScopedQitiTest test;
    
    auto dataRace = []()
    {
        TestClass testClass;
        
        std::thread t([&testClass](){ testClass.incrementCounter(); }); // Intentional data race
        testClass.incrementCounter();                                   // Intentional data race
        t.join();
    };
    auto dataRaceDetector = qiti::ThreadSanitizer::createDataRaceDetector();
    dataRaceDetector->run(dataRace);
    QITI_REQUIRE(dataRaceDetector->failed());
    QITI_REQUIRE_FALSE(dataRaceDetector->passed());
}
#endif

// TODO: remove when createPotentialDeadlockDetector() is fully implemented, not just on Apple
#if ! defined(__APPLE__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated"
#endif
TEST_CASE("qiti::ThreadSanitizer::createPotentialDeadlockDetector() does not produce false positive")
{
    qiti::ScopedQitiTest test;
    
    auto potentialDeadlockDetector = qiti::ThreadSanitizer::createPotentialDeadlockDetector();
    
    SECTION("Run code containing no locks.")
    {
        auto noMutexesAtAll = [](){};
        
        // Should pass
        potentialDeadlockDetector->run(noMutexesAtAll);
        QITI_REQUIRE(potentialDeadlockDetector->passed());
        QITI_REQUIRE_FALSE(potentialDeadlockDetector->failed());
    }
    
    SECTION("Run code containing 1 lock that does not deadlock.")
    {
        auto singleMutexWithNoDeadlock = []()
        {
            std::mutex mutex;
            
            // Mutex locked in parallel
            std::thread t([&mutex]
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
#if ! defined(__APPLE__)
#pragma clang diagnostic pop
#endif

#if defined(__APPLE__) // TODO: remove when this feature is supported on Linux
TEST_CASE("qiti::ThreadSanitizer::createPotentialDeadlockDetector() detects potential deadlock")
{
    qiti::ScopedQitiTest test;
    
    auto potentialDeadlockDetector = qiti::ThreadSanitizer::createPotentialDeadlockDetector();
    
    SECTION("Run code that inverts the order of mutex locking which implies a potential deadlock,"
            "but does not actually deadlock here.")
    {
        auto singleMutexWithNoDeadlock = []()
        {
            std::mutex mutexA;
            std::mutex mutexB;
            
            std::thread t([&]
            {
                // Thread t locks A then B
                std::lock_guard<std::mutex> lockA(mutexA);
                // Give main thread a chance to run
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                std::lock_guard<std::mutex> lockB(mutexB);
            });
            
            // Main thread locks B then A, but uses scoped_lock (deadlock-safe)
            std::scoped_lock lock(mutexB, mutexA);
            
            t.join();
        };
        
        // Should fail
        potentialDeadlockDetector->run(singleMutexWithNoDeadlock);
        QITI_REQUIRE_FALSE(potentialDeadlockDetector->passed());
        QITI_REQUIRE(potentialDeadlockDetector->failed());
    }
}
#endif // defined(__APPLE__)
