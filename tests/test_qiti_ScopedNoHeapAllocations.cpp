
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"
// Basic Catch2 macros
#include <catch2/catch_test_macros.hpp>

// Qiti Private API - not included in qiti_include.hpp
#include "qiti_ScopedNoHeapAllocations.hpp"

// Works in Xcode but not in the command line (for CI)
// so commenting out for now
//TEST_CASE("ScopedNoHeapAllocations aborts on unexpected heap alloc")
//{
//    QITI_REQUIRE_DEATH
//    (
//        qiti::ScopedNoHeapAllocations guard;
//        (void)new int;
//        // guard goes out of scope ⇒ assert fires ⇒ abort
//    );
//}

TEST_CASE("ScopedNoHeapAllocations survives no heap alloc")
{
    qiti::resetAll();
    
    // This should *not* abort.
    QITI_REQUIRE_SURVIVES
    (
        qiti::ScopedNoHeapAllocations guard;
        [[maybe_unused]] volatile auto dummy = 42; // stack alloc, not heap (safe)
    );
}
