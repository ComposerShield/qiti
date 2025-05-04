
#include "qiti_instrument.hpp"

#include "qiti_test_macros.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("qiti::onNextHeapAllocation() is called on next heap allocation", "[qiti::onNextHeapAllocation]")
{
    static int testValue = 0;
    qiti::instrument::onNextHeapAllocation([](){ ++testValue; });
    
    auto* heapAllocation = new int{0};
    QITI_CHECK(testValue == 1);
    delete heapAllocation;
    
    qiti::shutdown();
}
