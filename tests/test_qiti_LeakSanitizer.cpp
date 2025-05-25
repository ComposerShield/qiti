
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"
// Basic Catch2 macros
#include <catch2/catch_test_macros.hpp>

#include "qiti_LeakSanitizer.hpp"


TEST_CASE("qiti::LeakSanitizer::passed")
{
    qiti::ScopedQitiTest test;
    
    qiti::LeakSanitizer lsan;
    // no leaking code
    QITI_REQUIRE(lsan.passed());
}
