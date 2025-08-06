
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

#include "qiti_LeakSanitizer.hpp"

//--------------------------------------------------------------------------
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated" // TODO: remove when finished implementing
QITI_TEST_CASE("qiti::LeakSanitizer::passed", LeakSanitizerPassed)
{
    qiti::ScopedQitiTest test;
    
    qiti::LeakSanitizer lsan;
    // no leaking code
    QITI_REQUIRE(lsan.passed());
}
#pragma clang diagnostic pop
