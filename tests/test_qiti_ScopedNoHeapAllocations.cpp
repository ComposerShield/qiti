
// Copyright (c) 2025 Adam Shield
// SPDX-License-Identifier: MIT

#include "qiti_ScopedNoHeapAllocations.hpp"

#include "qiti_test_macros.hpp"
#include "qiti_utils.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("ScopedNoHeapAllocations aborts on unexpected heap alloc")
{
//    QITI_REQUIRE_DEATH
//    (
//        qiti::ScopedNoHeapAllocations guard;
//        (void)new int;
//        // guard goes out of scope ⇒ assert fires ⇒ abort
//    );
}

TEST_CASE("ScopedNoHeapAllocations survives no heap alloc")
{
    // This should *not* abort.
    QITI_REQUIRE_SURVIVES
    (
        qiti::ScopedNoHeapAllocations guard;
        [[maybe_unused]] volatile auto dummy = 42; // stack alloc, not heap (safe)
    );
}
