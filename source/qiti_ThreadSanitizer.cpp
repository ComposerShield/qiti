
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


#include "qiti_FunctionData.hpp"
#include "qiti_LockData.hpp"
#include "qiti_LockHooks.hpp"
#include "qiti_MallocHooks.hpp"
#include "qiti_Profile.hpp"

#include <string.h>     // for strsignal() - NOLINT(modernize-deprecated-headers) POSIX function not in <cstring>
#include <sys/types.h>  // required for wait.h

#if ! defined(_WIN32)
#include <sys/wait.h>   // for waitpid
#include <unistd.h>     // for fork()
#endif

#include <atomic>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <optional>
#include <ranges>       // NOLINT - false positive in cpplint
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

//--------------------------------------------------------------------------

[[maybe_unused]] static constexpr const char* QITI_TSAN_LOG_PATH = "/tmp/tsan.log";

namespace fs = std::filesystem;

/**
 Finds the most recently modified file matching the prefix.

 @param prefix The log file prefix to search for.
 @returns an optional containing the latest path, or nullopt if none found.
*/
[[maybe_unused]] [[nodiscard]] static std::optional<fs::path> findLatestLog(const std::string& prefix) noexcept
{
    std::optional<fs::path> best;
    fs::path dir = fs::path(prefix).parent_path();
    std::string base = fs::path(prefix).filename().string();
    for (auto& ent : fs::directory_iterator(dir))
    {
        auto fn = ent.path().filename().string();
        if (fn.starts_with(base))
        {
            if (!best || fs::last_write_time(ent) > fs::last_write_time(*best))
                best = ent.path();
        }
    }
    return best;
}

/**
 Read a file fully into a string.

 @param path Path of the file to read.
 @returns the file contents; empty string on failure.
*/
[[maybe_unused]] [[nodiscard]] static std::string slurpFile(const fs::path& path) noexcept
{
    std::ifstream in(path, std::ios::binary);
    return { std::istreambuf_iterator<char>(in),
             std::istreambuf_iterator<char>() };
}

//--------------------------------------------------------------------------
namespace qiti
{

#ifdef QITI_ENABLE_CLANG_THREAD_SANITIZER
//--------------------------------------------------------------------------
class DataRaceDetector final : public ThreadSanitizer
{
public:
    /** */
    QITI_API_INTERNAL DataRaceDetector() noexcept = default;
    /** */
    QITI_API_INTERNAL ~DataRaceDetector() noexcept override = default;
    
    void QITI_API_INTERNAL run(std::function<void()> func) noexcept override
    {
        qiti::Profile::ScopedDisableProfiling disableProfiling;
        qiti::LockHooks::ScopedDisableHooks disableHooks;
        
        // Cache function for rerun()
        _cachedFunction = func;
        
        // Reset state from previous runs
        shortReport.clear();
        verboseReport.clear();
        _passed.store(true, std::memory_order_relaxed);
        
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
        
        MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;
        
        // Parent: wait for child to exit (TSan will have exit()ed, flushing the log)
        int status = 0;
        {
            pid_t w;
            do
            {
                w = waitpid(pid, &status, 0);
            }
            while (w == -1 && errno == EINTR);
            
            assert(w == pid);
        }
        
        if (WIFEXITED(status))
        {
            auto statusCode = WEXITSTATUS(status);
            std::cout << "[qiti::DataRaceDetector] Child Status Code: " << statusCode << "\n";
        }
        else if (WIFSIGNALED(status))
        {
            // Child killed by signal
            int sig = WTERMSIG(status);
            std::cout << "[qiti::DataRaceDetector] Child killed by signal " << sig << " (" << strsignal(sig) << ")\n";
        }
        else
        {
            std::terminate(); // Child neither exited nor was signaled?
        }
        
        // Now read the newly created tsan.log.*
        auto logPath = findLatestLog(logPrefix);
        
        if (logPath.has_value()) // new TSan file was written
        {
            std::cout << "[qiti::DataRaceDetector] Reading TSan log at: " << *logPath << "\n";
            verboseReport = slurpFile(*logPath);
            
            // Case‐insensitive "data race" search
            static const std::regex data_race_rx(R"(data race)",
                                                 std::regex_constants::icase);
            
            // Look for "data race" anywhere in the report
            if (std::regex_search(verboseReport, data_race_rx))
            {
                flagFailed();
                
                std::cout << "[qiti::DataRaceDetector] Data race detected!\n";

                static const std::regex summary_rx(R"(^.*SUMMARY:.*$)",
                                                   std::regex_constants::multiline);
                std::smatch sm;
                if (std::regex_search(verboseReport, sm, summary_rx))
                    shortReport = sm.str();
                else
                    shortReport = "No SUMMARY found.";
                
                std::cout << "[qiti::DataRaceDetector] " << shortReport << "\n";
            }
            else
            {
                std::cout << "[qiti::DataRaceDetector] No data race detected.\n";
            }
        }
        else
        {
            std::cout << "[qiti::DataRaceDetector] No TSan log produced. Likely no data race detected.\n";
        }
        
        // cleanup log that was created
        std::system(("rm -f " + std::string(logPrefix) + "*").c_str());
    }
    
    std::string QITI_API_INTERNAL getReport(bool verbose) const noexcept override
    {
        return verbose ? verboseReport : shortReport;
    }
    
private:
    std::string shortReport{};
    std::string verboseReport{};
};

#endif // QITI_ENABLE_CLANG_THREAD_SANITIZER

//--------------------------------------------------------------------------

/** Detects whether two functions ever run in parallel. */
class ParallelCallDetector final
: public ThreadSanitizer
, private FunctionData::Listener
{
public:
    /** */
    QITI_API_INTERNAL ParallelCallDetector(FunctionData* _func0,
                                           FunctionData* _func1) noexcept
    : func0(_func0)
    , func1(_func1)
    {}
    
    /** */
    QITI_API_INTERNAL ~ParallelCallDetector() noexcept override = default;
    
    void QITI_API_INTERNAL run(std::function<void()> func) noexcept override
    {
        // Disable profiling for setup
        {
            qiti::Profile::ScopedDisableProfiling disableProfiling;
            
            // Cache function for rerun()
            _cachedFunction = func;
            
            // Reset state from previous runs
            shortReport.clear();
            verboseReport.clear();
            _passed.store(true, std::memory_order_relaxed);
            numConcurrentFunc0.store(0, std::memory_order_relaxed);
            numConcurrentFunc1.store(0, std::memory_order_relaxed);
            
            func0->addListener(this);
            func1->addListener(this);
        }
        
        // Call user function with profiling enabled
        func();
        
        // Disable profiling for cleanup
        {
            qiti::Profile::ScopedDisableProfiling disableProfiling;
            func0->removeListener(this);
            func1->removeListener(this);
        }
    }
    
    std::string QITI_API_INTERNAL getReport(bool verbose) const noexcept override
    {
        return verbose ? verboseReport : shortReport;
    }
    
private:
    FunctionData* const func0;
    FunctionData* const func1;
    
    std::atomic<int32_t> numConcurrentFunc0 = 0;
    std::atomic<int32_t> numConcurrentFunc1 = 0;
    
    using MutexType = std::mutex;
    using LockType = std::scoped_lock<MutexType>;
    MutexType reportLock{};
    
    std::string shortReport{};
    std::string verboseReport{};
    
    static constexpr const char* firstFuncCalledWhileInSecondString
        = "1st function called while 2nd function was running.";
    static constexpr const char* secondFuncCalledWhileInFirstString
        = "2nd function called while 1st function was running.";
    
    void QITI_API_INTERNAL onFunctionEnter(const FunctionData* func) noexcept override
    {
        assert(func == func0 || func == func1);
        
        auto& numConcurrent            = (func == func0) ? numConcurrentFunc0 : numConcurrentFunc1;
        const auto& numConcurrentOther = (func == func0) ? numConcurrentFunc1 : numConcurrentFunc0;
        
        if (numConcurrentOther > 0) // other func is currently running
        {
            flagFailed();
            
            MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;
            
            qiti::LockHooks::LockBypassingHook<LockType, MutexType> lock(reportLock);
            
            // Only log first infraction into shortReport
            if (shortReport.empty())
            {
                shortReport.append((func == func0) ? firstFuncCalledWhileInSecondString
                                                   : secondFuncCalledWhileInFirstString);
                shortReport.append(" (Subsequent infractions ignored)");
            }
            
            // Log every infraction into verboseReport
            verboseReport.append((func == func0) ? firstFuncCalledWhileInSecondString
                                                 : secondFuncCalledWhileInFirstString);
            verboseReport.append("\n");
        }
        
        ++numConcurrent;
    }
    
    void QITI_API_INTERNAL onFunctionExit (const FunctionData* func) noexcept override
    {
        auto& numConcurrent = (func == func0) ? numConcurrentFunc0 : numConcurrentFunc1;
        --numConcurrent;
    }    
};

//--------------------------------------------------------------------------
/** Detects potential deadlocks by watching acquire‐order inversions. */
class LockOrderInversionDetector final
: public ThreadSanitizer
, private LockData::Listener
{
public:
    /** */
    QITI_API_INTERNAL LockOrderInversionDetector() noexcept = default;
    /** */
    QITI_API_INTERNAL ~LockOrderInversionDetector() noexcept override = default;
    
    void QITI_API_INTERNAL run(std::function<void()> func) noexcept override
    {
        // Disable profiling for setup
        {
            qiti::Profile::ScopedDisableProfiling disableProfiling;
            
            // Cache function for rerun()
            _cachedFunction = func;
            
            // Reset state from previous runs
            _passed.store(true, std::memory_order_relaxed);
            {
                std::lock_guard _(_graphLock);
                _edges.clear();
            }
            _heldStack.clear();
            
            LockData::addGlobalListener(this);
        }
        
        // Call user function with profiling enabled
        func();
        
        // Disable profiling for cleanup
        {
            qiti::Profile::ScopedDisableProfiling disableProfiling;
            LockData::removeGlobalListener(this);
        }
    }

private:
    // Global lock‐order graph: edge A → B means "A was held when B was acquired".
    // We only need to detect any cycle.
    std::mutex _graphLock;
    std::unordered_map<const void*, std::vector<const void*>> _edges;

    // Per-thread stack of held locks:
    inline static thread_local std::vector<const void*> _heldStack;

    // Listener callbacks:
    void QITI_API_INTERNAL onAcquire(const pthread_mutex_t* mutexAddress) noexcept override
    {
        auto key = reinterpret_cast<const void*>(mutexAddress);
        
        // Check for cycle: if any held H has a path back to itself via key
        {
            std::lock_guard _(_graphLock);
            for (const void* H : _heldStack)
            {
                // add edge H→key, but first see if key→…→H already exists
                if (_detectPath(key, H))
                {
                    flagFailed();
                }
                _edges[H].push_back(key);
            }
        }
        
        // Push this lock on our held stack
        _heldStack.push_back(key);
    }

    void QITI_API_INTERNAL onRelease(const pthread_mutex_t* mutexAddress) noexcept override
    {
        auto key = reinterpret_cast<const void*>(mutexAddress);
        
        assert(! _heldStack.empty());
        
        // pop the stack (must be last)
        if (_heldStack.back() == key)
            _heldStack.pop_back();
        else
        {
            flagFailed(); // inverse order detected
            auto it = std::ranges::find(_heldStack, key);
            if (it != _heldStack.end())
                _heldStack.erase(it);
        }
    }

    // DFS to see if there's a path from 'from' to 'to' in our lock‐order graph
    bool QITI_API_INTERNAL _detectPath(const void* from, const void* to) noexcept
    {
        std::unordered_set<const void*> seen;
        std::vector<const void*> stack{from};
        while (! stack.empty())
        {
            const void* cur = stack.back(); stack.pop_back();
            if (cur == to)
                return true;
            if (!seen.insert(cur).second)
                continue;
            auto it = _edges.find(cur);
            if (it != _edges.end())
                for (const void* nxt : it->second)
                    stack.push_back(nxt);
        }
        return false;
    }
};

//--------------------------------------------------------------------------

#ifdef QITI_ENABLE_CLANG_THREAD_SANITIZER
/** Linux deadlock detector that uses TSan's built-in deadlock detection */
class TSanDeadlockDetector final : public ThreadSanitizer
{
public:
    QITI_API_INTERNAL TSanDeadlockDetector() noexcept = default;
    QITI_API_INTERNAL ~TSanDeadlockDetector() noexcept override = default;
    
    void QITI_API_INTERNAL run(std::function<void()> func) noexcept override
    {
        constexpr char const* logPrefix = QITI_TSAN_LOG_PATH;
        const char* oldTsanOptions;
        
        {
            qiti::Profile::ScopedDisableProfiling disableProfiling;
            qiti::LockHooks::ScopedDisableHooks disableHooks;
            
            // Cache function for rerun()
            _cachedFunction = func;
            
            // Reset state from previous runs
            shortReport.clear();
            verboseReport.clear();
            _passed.store(true, std::memory_order_relaxed);
            
            // Enable TSan deadlock detection for this run
            oldTsanOptions = getenv("TSAN_OPTIONS");
            std::string oldTsanOptionsStr;
            if (oldTsanOptions != nullptr)
            {
                oldTsanOptionsStr = oldTsanOptions;
            }
            std::string newTsanOptions = "detect_deadlocks=1:abort_on_error=0:log_path=";
            newTsanOptions += logPrefix;
            if (! oldTsanOptionsStr.empty())
            {
                newTsanOptions += ":";
                newTsanOptions += oldTsanOptionsStr;
            }
            setenv("TSAN_OPTIONS", newTsanOptions.c_str(), 1);
            
            // wipe any old logs
            std::system(("rm -f " + std::string(logPrefix) + "*").c_str());
            
        }
        
        // fork & exec the helper that runs the potential deadlock
        // Call user function with profiling enabled
        pid_t pid = fork();
        assert(pid >= 0);
        if (pid == 0)
        {
            // Child: run the function with TSan deadlock detection
            func();
            _exit(0);
        }
        
        // Parent: wait for child to exit
        int status = 0;
        {
            pid_t w;
            do
            {
                w = waitpid(pid, &status, 0);
            }
            while (w == -1 && errno == EINTR);
            assert(w == pid);
        }
        
        qiti::Profile::ScopedDisableProfiling disableProfiling;
        qiti::LockHooks::ScopedDisableHooks disableHooks;
        qiti::MallocHooks::ScopedBypassMallocHooks bypassMallocHooks;
        
        // Restore old TSAN_OPTIONS
        if (! oldTsanOptionsStr.empty())
        {
            setenv("TSAN_OPTIONS", oldTsanOptionsStr.c_str(), 1);
        }
        else
        {
            unsetenv("TSAN_OPTIONS");
        }
        
        if (WIFEXITED(status))
        {
            auto statusCode = WEXITSTATUS(status);
            std::cout << "[qiti::TSanDeadlockDetector] Child Status Code: " << statusCode << "\n";
        }
        else if (WIFSIGNALED(status))
        {
            int sig = WTERMSIG(status);
            std::cout << "[qiti::TSanDeadlockDetector] Child killed by signal " << sig << " (" << strsignal(sig) << ")\n";
        }
        else
        {
            std::terminate();
        }
        
        // Read the TSan log for deadlock reports
        auto logPath = findLatestLog(logPrefix);
        
        if (logPath.has_value())
        {
            std::cout << "[qiti::TSanDeadlockDetector] Reading TSan log at: " << *logPath << "\n";
            verboseReport = slurpFile(*logPath);
            
            // Look for deadlock-related messages
            static const std::regex deadlock_rx(R"(deadlock|lock.order.inversion|potential.deadlock)",
                                               std::regex_constants::icase);
            
            if (std::regex_search(verboseReport, deadlock_rx))
            {
                flagFailed();
                std::cout << "[qiti::TSanDeadlockDetector] Potential deadlock detected!\n";

                static const std::regex summary_rx(R"(^.*SUMMARY:.*$)",
                                                   std::regex_constants::multiline);
                std::smatch sm;
                if (std::regex_search(verboseReport, sm, summary_rx))
                    shortReport = sm.str();
                else
                    shortReport = "Potential deadlock detected (no SUMMARY found).";
                
                std::cout << "[qiti::TSanDeadlockDetector] " << shortReport << "\n";
            }
            else
            {
                std::cout << "[qiti::TSanDeadlockDetector] No deadlock detected.\n";
            }
        }
        else
        {
            std::cout << "[qiti::TSanDeadlockDetector] No TSan log produced. Likely no deadlock detected.\n";
        }
        
        // cleanup
        std::system(("rm -f " + std::string(logPrefix) + "*").c_str());
    }
    
    std::string QITI_API_INTERNAL getReport(bool verbose) const noexcept override
    {
        return verbose ? verboseReport : shortReport;
    }
    
private:
    std::string shortReport{};
    std::string verboseReport{};
};
#endif // QITI_ENABLE_CLANG_THREAD_SANITIZER

//--------------------------------------------------------------------------

ThreadSanitizer::ThreadSanitizer() noexcept = default;
ThreadSanitizer::~ThreadSanitizer() noexcept = default;

bool ThreadSanitizer::passed() noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
    return _passed.load(std::memory_order_relaxed);
}

bool ThreadSanitizer::failed() noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
    return ! passed();
}

void ThreadSanitizer::flagFailed() noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
    _passed.store(false, std::memory_order_seq_cst);
    if (onFail != nullptr)
        onFail();
}

std::unique_ptr<ThreadSanitizer>
ThreadSanitizer::createFunctionsCalledInParallelDetector(FunctionData* func0,
                                                         FunctionData* func1) noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
    return std::make_unique<ParallelCallDetector>(func0, func1);
}

#ifdef QITI_ENABLE_CLANG_THREAD_SANITIZER
std::unique_ptr<ThreadSanitizer>
ThreadSanitizer::createDataRaceDetector() noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
    return std::make_unique<DataRaceDetector>();
}
#endif // QITI_ENABLE_CLANG_THREAD_SANITIZER

#if defined(__APPLE__) || defined(QITI_ENABLE_CLANG_THREAD_SANITIZER)
std::unique_ptr<ThreadSanitizer>
ThreadSanitizer::createPotentialDeadlockDetector() noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
#if defined(__APPLE__)
    return std::make_unique<LockOrderInversionDetector>();
#else
    // Linux: Use TSan's built-in deadlock detection
#if defined(QITI_ENABLE_CLANG_THREAD_SANITIZER)
    return std::make_unique<TSanDeadlockDetector>();
#else
    return nullptr;
#endif // defined(QITI_ENABLE_CLANG_THREAD_SANITIZER)
#endif // defined(__APPLE__)
}
#endif // defined(__APPLE__) || defined(QITI_ENABLE_CLANG_THREAD_SANITIZER)

void ThreadSanitizer::rerun() noexcept
{
    // Do not disable profiling here since we want profiling when running the cached function
    
    if (_cachedFunction != nullptr)
        run(_cachedFunction);
}

std::string ThreadSanitizer::getReport(bool /*verbose*/) const noexcept
{
    qiti::Profile::ScopedDisableProfiling disableProfiling;
    return {};
};

//--------------------------------------------------------------------------
} // namespace qiti
//--------------------------------------------------------------------------
