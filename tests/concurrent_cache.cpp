#include "catch.hpp"

#include <iostream>
#include <set>

#include <cachew/concurrent_cache.hpp>

#include "common.hpp"

using namespace cachew;

TEST_CASE( "concurrent_cache base" )
{
    concurrent_cache<int, int> cache( 5 );

    cache.put( 1, 11 );
    cache.put( 2, 22 );
    cache.put( 3, 33 );

    CHECK( cache.size() == 3 );

    CHECK( cache.get( 2 ) == 22 );
}
