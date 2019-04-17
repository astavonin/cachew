#include "catch.hpp"

#include <iostream>
#include <set>

#include <cachew/lfu_cache.hpp>

using namespace cachew;

TEST_CASE( "LFU cache size" )
{
    lfu_cache<int, int> cache( 3 );

    for( int i = 0; i < 10; i++ )
    {
        cache.put( i, i * 10 );
    }

    CHECK(cache.get(8) == 80);
    cache.get(8);
    CHECK(cache.get(1) == 0);

    cache.dump_debug(std::cout);
}
