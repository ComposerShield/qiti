
#include "qiti_include.hpp"

#include "qiti_LeakSanitizer.hpp"

#include "qiti_test_macros.hpp"

#include <catch2/catch_test_macros.hpp>


TEST_CASE("qiti::LeakSanitizer::passed")
{
    qiti::resetAll();
    
    qiti::LeakSanitizer lsan;
    // no leaking code
    QITI_REQUIRE(lsan.passed());
    
    qiti::resetAll();
}
