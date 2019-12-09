#include "catch.hpp"

#include <iostream>
#include <set>

#include <cachew/lru_cache.hpp>

#include "common.hpp"

using namespace cachew;

TEST_CASE( "LRU cache benchmark", "[benchmark]" )
{
    SECTION( "POD types, 500 elements" )
    {
        const size_t        cache_size      = 500;
        const size_t        iteration_count = 10'000;
        lru_cache<int, int> cache( cache_size );

        BENCHMARK( "integer cache put (500, 10'000 iterations)" )
        {
            for( size_t i = 0; i < iteration_count; i++ )
            {
                cache.put( i, i );
            }
        };

        BENCHMARK( "integer cache get (10'000 iterations)" )
        {
            for( size_t i = 0; i < iteration_count; i++ )
            {
                cache.get( i );
            }
        };
    }
    SECTION( "POD types, 5000 elements" )
    {
        const size_t        cache_size      = 5000;
        const size_t        iteration_count = 10'000;
        lru_cache<int, int> cache( cache_size );

        BENCHMARK( "integer cache put (5000, 10'000 iterations)" )
        {
            for( size_t i = 0; i < iteration_count; i++ )
            {
                cache.put( i, i );
            }
        };

        BENCHMARK( "integer cache get (10'000 iterations)" )
        {
            for( size_t i = 0; i < iteration_count; i++ )
            {
                cache.get( i );
            }
        };
    }
    SECTION( "POD types, 50'000 elements" )
    {
        const size_t        cache_size      = 50'000;
        const size_t        iteration_count = 100'000;
        lru_cache<int, int> cache( cache_size );

        BENCHMARK( "integer cache put (50'000, 100'000 iterations)" )
        {
            for( size_t i = 0; i < iteration_count; i++ )
            {
                cache.put( i, i );
            }
        };

        BENCHMARK( "integer cache get (100'000 iterations)" )
        {
            for( size_t i = 0; i < iteration_count; i++ )
            {
                cache.get( i );
            }
        };
    }
}
