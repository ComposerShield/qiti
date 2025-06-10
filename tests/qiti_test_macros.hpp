
#include <cassert>
#include <cstring> // for std::strerror

#if (QITI_CATCH2==1)
    #include <catch2/catch_test_macros.hpp>
#elif (QITI_GTEST==1)
    #include <gtest/gtest.h>
#endif

#if defined (DEBUG) || defined (_DEBUG) || ! (defined (NDEBUG) || defined (_NDEBUG))
    #define QITI_DEBUG_ASSERT(expression) assert(expression)
#else
    #define QITI_DEBUG_ASSERT(expression)
#endif // #if defined (DEBUG) || defined (_DEBUG)

// Unit Tests Macro Extensions
// Note: the do/while loop design is important to maintain the expression as a single statement

#if (QITI_CATCH2==1)
    #define QITI_TEST(funcName, funcDescription) TEST_CASE( #funcName, #funcDescription )
    #define QITI_SECTION SECTION

    // Extend Catch2 CHECK test to assert inline in debug builds
    #define QITI_CHECK(...) do {                          \
        QITI_DEBUG_ASSERT(__VA_ARGS__); /* Debug assert inline */ \
        CHECK(__VA_ARGS__);   /* Forward to Catch2  */  \
    } while (false)
    
    // Extend Catch2 REQUIRE test to assert inline in debug builds
    #define QITI_REQUIRE(...) do {                        \
        QITI_DEBUG_ASSERT(__VA_ARGS__); /* Debug assert inline */ \
        REQUIRE(__VA_ARGS__); /* Forward to Catch2  */  \
    } while (false)
    
    // Extend Catch2 REQUIRE_FALSE test to assert inline in debug builds
    #define QITI_REQUIRE_FALSE(...) do {                        \
        QITI_DEBUG_ASSERT(! (__VA_ARGS__));   /* Debug assert inline */ \
        REQUIRE_FALSE(__VA_ARGS__); /* Forward to Catch2  */  \
    } while (false)

#elif (QITI_GTEST==1)
    #define QITI_TEST TEST
    #define QITI_SECTION(...)

    #define QITI_CHECK EXPECT_TRUE
    
    #define QITI_REQUIRE ASSERT_TRUE
    
    #define QITI_REQUIRE_FALSE ASSERT_FALSE
#endif // (QITI_CATCH2==1)

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
