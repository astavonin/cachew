#include "catch.hpp"

#include <iostream>
#include <set>

#include <cachew/lfu_cache.hpp>

using namespace cachew;

template <class T>
std::set<T> to_set( const lfu_cache<T, T> cache )
{
    std::set<int> res;
    for( auto val : cache )
    {
        res.emplace( val );
    }
    return res;
}

TEST_CASE( "LFU cache size" )
{
    static const size_t cache_size = 3;
    lfu_cache<int, int> cache( cache_size );

    for( int i = 0; i < 10; i++ )
    {
        cache.put( i, i * 10 );
    }
    REQUIRE( cache.size() == cache_size );

    auto val = cache.get( 8 );
    REQUIRE( val != cache.end() );

    CHECK( to_set( cache ) == std::set<int>{70, 80, 90} );

    val = cache.get( 1 );
    CHECK( val == cache.end() );

    cache.put( 1, 10 );
    cache.put( 2, 20 );
    CHECK( to_set( cache ) == std::set<int>{10, 20, 80} );
}

TEST_CASE( "LFU iterator" )
{
    lfu_cache<int, int> cache( 5 );

    cache.put( 1, 11 );
    cache.put( 2, 22 );
    cache.put( 3, 33 );

    CHECK( cache.begin() != cache.end() );

    auto it = cache.get( 42 );
    CHECK( it == cache.end() );

    it = cache.get( 1 );
    CHECK( it != cache.end() );

    std::set<int> expected = {11, 22, 33};
    CHECK( to_set( cache ) == expected );
}

TEST_CASE( "LFU ctors and assignment" )
{
    lfu_cache<int, int> cache( 5 );

    std::set<int> expected = {50, 60, 70, 80, 90};

    for( int i = 0; i < 10; i++ )
    {
        cache.put( i, i * 10 );
    }

    SECTION( "ctors" )
    {
        lfu_cache<int, int> cache_new( cache );

        CHECK( to_set( cache ) == expected );
    }

    SECTION( "assignment" )
    {
        lfu_cache<int, int> cache_new( 5 );

        cache_new = cache;

        CHECK( to_set( cache_new ) == expected );
    }

    SECTION( "swap" )
    {
        lfu_cache<int, int> cache_new( 5 );

        std::swap( cache_new, cache );

        CHECK( cache.size() == 0 );
        CHECK( cache_new.size() == 5 );
        CHECK( to_set( cache_new ) == expected );
    }
}
