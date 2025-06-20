
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"
// Basic Catch2 macros
#include <catch2/catch_test_macros.hpp>

// Qiti Private API - not included in qiti_include.hpp
#include "qiti_CoverageHooks.hpp"

#include <filesystem>

//--------------------------------------------------------------------------


//--------------------------------------------------------------------------

TEST_CASE("qiti::CoverageHooks::reset")
{
    qiti::ScopedQitiTest test;
    
    // Write current coverage to .gcda files
    qiti::CoverageHooks::dump();
    
    {
        bool folderExists = std::filesystem::exists("/tmp/coverage");
        QITI_REQUIRE(folderExists);
    }
    
    // Should delete .gcda files and reset internal counters
    qiti::CoverageHooks::reset();
    
    {
        bool folderExists = std::filesystem::exists("/tmp/coverage");
        QITI_REQUIRE_FALSE(folderExists);
    }
}
