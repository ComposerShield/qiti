
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

// Qiti Private API - not included in qiti_include.hpp
#include "qiti_LockHooks.hpp"
#include "qiti_ScopedNoHeapAllocations.hpp"

//--------------------------------------------------------------------------

#if ! defined(_WIN32)

// "Death Tests"

#include <sys/types.h>  // required for wait.h
#include <sys/wait.h>   // for waitpid
#include <unistd.h>     // for fork()

#include <csignal>
#include <functional>

/** Return true if f() causes the child to abort with SIGABRT. */
namespace qiti
{
[[nodiscard]] inline bool died_by_sigabrt(std::function<void()> f) noexcept(false) /** throws */
{
    LockHooks::ScopedDisableHooks disableHooks;
    
    pid_t pid = fork();
    if (pid < 0)
        throw std::runtime_error(std::string("fork() failed: ") + std::strerror(errno));
    
    if (pid == 0)
    {
        // Child: detach from parent PG so abort() only hits us
        if (setsid() < 0)
            _exit(EXIT_FAILURE);
        
        f();
        _exit(EXIT_SUCCESS);  // no abort == "survived"
    }
    
    // Parent: wait for the child
    int status = 0;
    while (true)
    {
        pid_t w = waitpid(pid, &status, 0);
        if (w < 0)
        {
            if (errno == EINTR) continue;
            throw std::runtime_error(std::string("waitpid() failed: ") + std::strerror(errno));
        }
        break;
    }
    return WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT;
}
} // namespace qiti

//--------------------------------------------------------------------------

#define QITI_REQUIRE_DEATH(...)                                     \
do {                                                                \
    bool _qiti_died = qiti::died_by_sigabrt([&](){ __VA_ARGS__; }); \
    QITI_REQUIRE(_qiti_died);                                       \
} while(false)

#define QITI_REQUIRE_SURVIVES(...)                                  \
do {                                                                \
    bool _qiti_died = qiti::died_by_sigabrt([&](){ __VA_ARGS__; }); \
    QITI_REQUIRE(! _qiti_died);                                     \
} while(false)

// Fails in CI but works locally...
//QITI_TEST_CASE("ScopedNoHeapAllocations aborts on unexpected heap alloc")
//{
//    qiti::ScopedQitiTest test;
//
//    QITI_REQUIRE_DEATH
//    (
//        qiti::ScopedNoHeapAllocations guard;
//        (void)new int;
//        // guard goes out of scope ⇒ assert fires ⇒ abort
//    );
//}

QITI_TEST_CASE("ScopedNoHeapAllocations survives no heap alloc", ScopedNoHeapAllocationsSurvives)
{
    qiti::ScopedQitiTest test;
    test.setMaximumDurationOfTest_ms(500ull);
    
    // This should *not* abort.
    QITI_REQUIRE_SURVIVES
    (
        qiti::ScopedNoHeapAllocations guard;
        [[maybe_unused]] volatile auto dummy = 42; // stack alloc, not heap (safe)
    );
}

#endif // ! defined(_WIN32)
