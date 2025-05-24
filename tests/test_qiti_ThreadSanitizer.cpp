
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

// Qiti Private API - not included in qiti_include.hpp
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
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
    qiti::resetAll();
    
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
    
    // Functions called in parallel
    std::thread t([]
    {
        for (auto i=0; i<1'000'000; ++i)
            testFunc0();
    });
    
    for (auto i=0; i<1'000'000; ++i)
        testFunc1(); // should race against thread t
    
    t.join();
    
    QITI_CHECK(! tsan->passed());
    QITI_CHECK(tsan->failed());
}

TEST_CASE("Detect data race via subprocess")
{
    qiti::resetAll();
    
    constexpr char const* logPrefix = "/tmp/tsan.log";

    // wipe any old logs
    std::system(("rm -f " + std::string(logPrefix) + "*").c_str());

    // fork & exec the helper that runs the race
    pid_t pid = fork();
    QITI_REQUIRE(pid >= 0);
    if (pid == 0)
    {
        // Child: drop into the helper binary (built alongside qiti_tests)
        std::thread t(incrementCounter); // Intentional data race
        incrementCounter();              // Intentional data race
        t.join();
        _exit(0); // clean exit of child process, may signal due to TSan
    }

    // Parent: wait for child to exit (TSan will have exit()ed, flushing the log)
    int status = 0;
    pid_t w;
    do
    {
        w = waitpid(pid, &status, 0);
    }
    while (w == -1 && errno == EINTR);

    QITI_CHECK(w == pid);
    
    if (WIFEXITED(status))
    {
        INFO("Child exited with code " << WEXITSTATUS(status));
        QITI_CHECK(WEXITSTATUS(status) != 0);  // expect non-zero on race
    }
    else if (WIFSIGNALED(status))
    {
        int sig = WTERMSIG(status);
        INFO("Child killed by signal " << sig);
        QITI_CHECK(sig == SIGTRAP);            // or allow SIGABRT if you switch to abort()
    }
    else
    {
        FAIL("Child neither exited nor was signaled?");
    }
    
    [[maybe_unused]] auto exitStatus = WEXITSTATUS(status);
//    QITI_REQUIRE(exitStatus == 0); // Causes crash in CI

    // Now read the newly created tsan.log.*
    auto logPath = findLatestLog(logPrefix);
    QITI_REQUIRE(logPath.has_value());
    std::string report = slurpFile(*logPath);

    // Report should mention the data race
    std::smatch m;
    std::regex rx(R"(global '([^']+)')");
    QITI_REQUIRE(std::regex_search(report, m, rx));
    QITI_CHECK(std::string(m[1]) == "counter");
}

TEST_CASE("qiti::ThreadSanitizer::createDataRaceDetector() does not produce false positive")
{
    qiti::resetAll();
    
    auto noDataRace = [](){};
    auto dataRaceDetector = qiti::ThreadSanitizer::createDataRaceDetector(noDataRace);
    QITI_REQUIRE(dataRaceDetector->passed());
    QITI_REQUIRE_FALSE(dataRaceDetector->failed());
}

TEST_CASE("qiti::ThreadSanitizer::createDataRaceDetector() detects data race of global variable")
{
    qiti::resetAll();
    
    auto dataRace = []()
    {
        std::thread t(incrementCounter); // Intentional data race
        incrementCounter();              // Intentional data race
        t.join();
    };
    auto dataRaceDetector = qiti::ThreadSanitizer::createDataRaceDetector(dataRace);
    QITI_REQUIRE(dataRaceDetector->failed());
    QITI_REQUIRE_FALSE(dataRaceDetector->passed());
}

TEST_CASE("qiti::ThreadSanitizer::createDataRaceDetector() detects data race of member variable")
{
    qiti::resetAll();
    
    auto dataRace = []()
    {
        TestClass testClass;
        
        std::thread t([&testClass](){ testClass.incrementCounter(); }); // Intentional data race
        testClass.incrementCounter();                                   // Intentional data race
        t.join();
    };
    auto dataRaceDetector = qiti::ThreadSanitizer::createDataRaceDetector(dataRace);
    QITI_REQUIRE(dataRaceDetector->failed());
    QITI_REQUIRE_FALSE(dataRaceDetector->passed());
}
