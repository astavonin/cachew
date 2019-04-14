#include "catch.hpp"

#include <iostream>
#include <set>

#include <cachew/lru_cache.hpp>

using namespace cachew;

template <class T>
std::set<T> to_set( const lru_cache<T, T> cache )
{
    std::set<int> res;
    for( auto val : cache )
    {
        res.emplace( val );
    }
    return res;
}

TEST_CASE( "iterator" )
{
    lru_cache<int, int> cache( 5 );

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

TEST_CASE( "cache size" )
{
    lru_cache<int, int> cache( 5 );

    CHECK( cache.size() == 0 );
    CHECK( cache.capacity() == 5 );

    for( int i = 0; i < 10; i++ )
    {
        cache.put( i, i * 10 );
    }

    CHECK( cache.size() == 5 );


    std::set<int> expected = {50, 60, 70, 80, 90};
    CHECK( to_set( cache ) == expected );

    cache.put( 7, 77 );
    cache.put( 1, 11 );

    CHECK( to_set( cache ) == std::set<int>{11, 60, 77, 80, 90} );
}

TEST_CASE( "ctors and assignment" )
{
    lru_cache<int, int> cache( 5 );

    std::set<int> expected = {50, 60, 70, 80, 90};

    for( int i = 0; i < 10; i++ )
    {
        cache.put( i, i * 10 );
    }

    SECTION( "ctors" )
    {
        lru_cache<int, int> cache_new( cache );

        CHECK( to_set( cache ) == expected );
    }

    SECTION( "assignment" )
    {
        lru_cache<int, int> cache_new( 5 );

        cache_new = cache;

        CHECK( to_set( cache_new ) == expected );
    }

    SECTION( "swap" )
    {
        lru_cache<int, int> cache_new( 5 );

        std::swap( cache_new, cache );

        CHECK( cache.size() == 0 );
        CHECK( cache_new.size() == 5 );
        CHECK( to_set( cache_new ) == expected );
    }
}