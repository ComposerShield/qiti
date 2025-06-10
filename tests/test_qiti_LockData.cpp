
// Example project
#include "qiti_example_include.hpp"
// Qiti Public API
#include "qiti_include.hpp"
// Special unit test include
#include "qiti_test_macros.hpp"

#include "qiti_LockData.hpp"

//--------------------------------------------------------------------------

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

QITI_TEST(LockData, delivers_acquire_and_release_to_a_single_listener)
{
    qiti::ScopedQitiTest test;

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

QITI_TEST(LockData, handle_multiple_listeners)
{
    qiti::ScopedQitiTest test;

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

QITI_TEST(LockData, key_returns_the_object_address_and_is_unique)
{
    qiti::ScopedQitiTest test;

    qiti::LockData a;
    qiti::LockData b;

    // key() should equal the object's address
    QITI_REQUIRE( a.key() == static_cast<const void*>(&a) );
    QITI_REQUIRE( b.key() == static_cast<const void*>(&b) );

    // each instance must produce a distinct key
    QITI_REQUIRE( a.key() != b.key() );

    // repeated calls stay consistent
    QITI_REQUIRE( a.key() == a.key() );
}
