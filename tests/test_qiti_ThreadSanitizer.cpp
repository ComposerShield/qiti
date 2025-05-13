
#include "qiti_include.hpp"
#include "qiti_ThreadSanitizer.hpp"
#include "qiti_test_macros.hpp"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <regex>

/** NOT static to purposely allow external linkage and visibility to QITI */
__attribute__((noinline)) __attribute__((optnone))
void testFunc_ThreadSanitizer0() noexcept
{
    volatile int _ = 42;
}

/** NOT static to purposely allow external linkage and visibility to QITI */
__attribute__((noinline)) __attribute__((optnone))
void testFunc_ThreadSanitizer1() noexcept
{
    volatile int _ = 42;
}

int counter = 0; // Shared global variable

__attribute__((noinline)) __attribute__((optnone))
static void incrementInThread()
{
    for (int i = 0; i < 1'000'000; ++i)
    {
        ++counter; // Unsynchronized write
    }
}

TEST_CASE("qiti::ThreadSanitizer::functionsNotCalledInParallel")
{
    qiti::resetAll();
    
    auto tsan = qiti::ThreadSanitizer::functionsNotCalledInParallel<testFunc_ThreadSanitizer0, testFunc_ThreadSanitizer1>();
    
    QITI_REQUIRE(tsan.passed());

    qiti::resetAll();
}

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

TEST_CASE("Detect data race via subprocess")
{
    constexpr char const* logPrefix = "/tmp/tsan.log";

    // wipe any old logs
    std::system(("rm -f " + std::string(logPrefix) + "*").c_str());

    // fork & exec the helper that runs the race
    pid_t pid = fork();
    QITI_REQUIRE(pid >= 0);
    if (pid == 0)
    {
        // Child: drop into the helper binary (built alongside qiti_tests)
        std::thread t(incrementInThread); // Intentional data race
        incrementInThread();              // Intentional data race
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
    
    QITI_REQUIRE(WEXITSTATUS(status) == 0);

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
