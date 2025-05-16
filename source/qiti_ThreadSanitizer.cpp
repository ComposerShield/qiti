
/******************************************************************************
 * Qiti — C++ Profiling Library
 *
 * @file     qiti_ThreadSanitizer.cpp
 *
 * @author   Adam Shield
 * @date     2025-05-16
 *
 * @copyright (c) 2025 Adam Shield
 * SPDX-License-Identifier: MIT
 *
 * See LICENSE.txt for license terms.
 ******************************************************************************/

#include "qiti_ThreadSanitizer.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

#include <unistd.h> // for fork()

//--------------------------------------------------------------------------

#define QITI_TSAN_LOG_PATH "/tmp/tsan.log"
static constexpr const char TSAN_DEFAULT_OPTS[] =
    "report_thread_leaks=0:abort_on_error=0:log_path=" QITI_TSAN_LOG_PATH;

extern "C" const char* __tsan_default_options()
{
    return TSAN_DEFAULT_OPTS;
}

namespace fs = std::filesystem;

[[nodiscard]] static std::optional<fs::path> findLatestLog(const std::string& prefix) noexcept
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

[[nodiscard]] static std::string slurpFile(const fs::path& p) noexcept
{
    std::ifstream in(p, std::ios::binary);
    return { std::istreambuf_iterator<char>(in),
             std::istreambuf_iterator<char>() };
}

//--------------------------------------------------------------------------

namespace qiti
{
class DataRaceDetector : public ThreadSanitizer
{
public:
    QITI_API DataRaceDetector(std::function<void()> func) noexcept
    {
        constexpr char const* logPrefix = QITI_TSAN_LOG_PATH;

        // wipe any old logs
        std::system(("rm -f " + std::string(logPrefix) + "*").c_str());

        // fork & exec the helper that runs the race
        pid_t pid = fork();
        assert(pid >= 0);
        if (pid == 0)
        {
            // Child: drop into the helper binary
            func();   // run the function in child process, scannning for data races
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

        assert(w == pid);
        
        if (WIFEXITED(status))
        {
            [[maybe_unused]] auto statusCode = WEXITSTATUS(status);
//            assert(statusCode != 0);  // expect non-zero on race
        }
        else if (WIFSIGNALED(status))
        {
            // Child killed by signal
            int sig = WTERMSIG(status);
            assert(sig == SIGTRAP);            // or allow SIGABRT if you switch to abort()
        }
        else
        {
            std::terminate(); // Child neither exited nor was signaled?
        }
        
        [[maybe_unused]] auto exitStatus = WEXITSTATUS(status);
    //    assert(exitStatus == 0); // Causes crash in CI

        // Now read the newly created tsan.log.*
        auto logPath = findLatestLog(logPrefix);
        if (logPath.has_value()) // new TSan file was written
        {
            _passed = false; // Should be nothing
            
            std::string report = slurpFile(*logPath);
            
            std::cout << "Data race detected!" << "\n" << "TSan report:" << "\n\n";
            std::cout << report << "\n";
            
            // Report should mention the data race
//            std::smatch m;
//            std::regex rx(R"(global '([^']+)')");
//            assert(std::regex_search(report, m, rx));
//            assert(std::string(m[1]) == "counter");
        }
        
        // cleanup log that was created
        std::system(("rm -f " + std::string(logPrefix) + "*").c_str());
    }
    
    QITI_API ~DataRaceDetector() final = default;
    
private:
    bool _passed = true;
    
    [[nodiscard]] bool QITI_API passed() noexcept final { return _passed; }
};

ThreadSanitizer::ThreadSanitizer() noexcept = default;
ThreadSanitizer::~ThreadSanitizer() noexcept = default;

bool ThreadSanitizer::passed() noexcept { return true; }
bool ThreadSanitizer::failed() noexcept { return ! passed(); }

ThreadSanitizer ThreadSanitizer::functionsNotCalledInParallel(void* /*func0*/, void* /*func1*/) noexcept
{
    return {}; // TODO: implement
}

std::unique_ptr<ThreadSanitizer> ThreadSanitizer::createDataRaceDetector(std::function<void()> func) noexcept
{
    return std::make_unique<DataRaceDetector>(func);
}

} // namespace qiti
