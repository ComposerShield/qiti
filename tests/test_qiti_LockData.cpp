
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"
// Basic Catch2 macros
#include <catch2/catch_test_macros.hpp>

#include "qiti_LockData.hpp"

/** Simple listener that records the last lock it saw acquired/released */
class TestListener : public qiti::LockData::Listener
{
public:
    const qiti::LockData* lastAcquire = nullptr;
    const qiti::LockData* lastRelease = nullptr;

    void onAcquire(const qiti::LockData* ld) noexcept override
    {
        lastAcquire = ld;
    }

    void onRelease(const qiti::LockData* ld) noexcept override
    {
        lastRelease = ld;
    }
};

TEST_CASE( "LockData delivers acquire and release to a single listener", "[LockData]" )
{
    qiti::resetAll();

    qiti::LockData ld;
    TestListener listener;

    qiti::LockData::addGlobalListener( &listener );

    ld.notifyAcquire();
    QITI_REQUIRE( listener.lastAcquire == &ld );

    ld.notifyRelease();
    QITI_REQUIRE( listener.lastRelease == &ld );

    qiti::LockData::removeGlobalListener( &listener );

    listener.lastAcquire = nullptr;
    listener.lastRelease = nullptr;

    ld.notifyAcquire();
    ld.notifyRelease();

    QITI_REQUIRE( listener.lastAcquire == nullptr );
    QITI_REQUIRE( listener.lastRelease == nullptr );
}

TEST_CASE( "LockData can handle multiple listeners", "[LockData]" )
{
    qiti::resetAll();

    qiti::LockData ld;
    TestListener a;
    TestListener b;

    qiti::LockData::addGlobalListener( &a );
    qiti::LockData::addGlobalListener( &b );

    ld.notifyAcquire();
    QITI_REQUIRE( a.lastAcquire == &ld );
    QITI_REQUIRE( b.lastAcquire == &ld );

    ld.notifyRelease();
    QITI_REQUIRE( a.lastRelease == &ld );
    QITI_REQUIRE( b.lastRelease == &ld );
}

TEST_CASE( "LockData::key() returns the object’s own address and is unique", "[LockData]" )
{
    qiti::resetAll();

    qiti::LockData a;
    qiti::LockData b;

    // key() should equal the object's address
    REQUIRE( a.key() == static_cast<const void*>(&a) );
    REQUIRE( b.key() == static_cast<const void*>(&b) );

    // each instance must produce a distinct key
    REQUIRE( a.key() != b.key() );

    // repeated calls stay consistent
    REQUIRE( a.key() == a.key() );
}
