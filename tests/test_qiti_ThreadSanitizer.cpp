
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"
// Basic Catch2 macros
#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <regex>

using namespace qiti::example::ThreadSanitizer;
namespace fs = std::filesystem;

[[nodiscard]] static std::optional<fs::path> findLatestLog(const std::string& prefix)
{
    std::optional<fs::path> best;
    fs::path dir = fs::path(prefix).parent_path();
    std::string base = fs::path(prefix).filename().string();
    for (auto& ent : fs::directory_iterator(dir))
    {
        auto fn = ent.path().filename().string();
        if (fn.rfind(base, 0) == 0)
        {
            if (!best || fs::last_write_time(ent) > fs::last_write_time(*best))
                best = ent.path();
        }
    }
    return best;
}

[[nodiscard]] static std::string slurpFile(const fs::path& p)
{
    std::ifstream in(p, std::ios::binary);
    return { std::istreambuf_iterator<char>(in),
             std::istreambuf_iterator<char>() };
}

//--------------------------------------------------------------------------

TEST_CASE("qiti::ThreadSanitizer::functionsNotCalledInParallel")
{
    qiti::ScopedQitiTest test;
    test.setMaximumDurationOfTest_ms(1000ull);
    
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
}

TEST_CASE("qiti::ThreadSanitizer::createDataRaceDetector() does not produce false positive")
{
    qiti::ScopedQitiTest test;
    test.permitLongTest();
    
    auto noDataRace = [](){};
    auto dataRaceDetector = qiti::ThreadSanitizer::createDataRaceDetector();
    dataRaceDetector->run(noDataRace);
    QITI_REQUIRE(dataRaceDetector->passed());
    QITI_REQUIRE_FALSE(dataRaceDetector->failed());
}

TEST_CASE("qiti::ThreadSanitizer::createDataRaceDetector() detects data race of global variable")
{
    qiti::ScopedQitiTest test;
    test.permitLongTest();
    
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
}

TEST_CASE("qiti::ThreadSanitizer::createDataRaceDetector() detects data race of member variable")
{
    qiti::ScopedQitiTest test;
    test.permitLongTest();
    
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

TEST_CASE("qiti::ThreadSanitizer::createPotentialDeadlockDetector() does not produce false positive")
{
    qiti::ScopedQitiTest test;
    
    auto potentialDeadlockDetector = qiti::ThreadSanitizer::createPotentialDeadlockDetector();
    
    SECTION("Run code containing no locks.")
    {
        auto noMutexesAtAll = [](){};
        
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
        
        potentialDeadlockDetector->run(singleMutexWithNoDeadlock);
        QITI_REQUIRE(potentialDeadlockDetector->passed());
        QITI_REQUIRE_FALSE(potentialDeadlockDetector->failed());
    }
}
