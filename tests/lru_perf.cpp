#include "catch.hpp"

#include <iostream>
#include <set>

#include <cachew/lru_cache.hpp>

#include "common.hpp"

using namespace cachew;

TEST_CASE( "LRU cache benchmark", "[benchmark]" )
{
    SECTION( "POD types" )
    {
        lru_cache<int, int> cache( 5000 );

        BENCHMARK( "integer cache put" )
        {
            for( int i = 0; i < 100000; i++ )
            {
                cache.put( i, i * 10 );
            }
        };

        BENCHMARK( "integer cache get" )
        {
            for( int i = 0; i < 100000; i++ )
            {
                cache.get( i );
            }
        };
    }
}
