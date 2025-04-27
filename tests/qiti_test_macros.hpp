
#include <cassert>

#if defined (DEBUG) || defined (_DEBUG) || ! (defined (NDEBUG) || defined (_NDEBUG))
    #define QITI_DEBUG_ASSERT(expression) assert(expression)
#else
    #define QITI_DEBUG_ASSERT(expression)
#endif // #if defined (DEBUG) || defined (_DEBUG)

// Unit Tests Macro Extensions
// Note: the do/while loop design is important to maintain the expression as a single statement

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
