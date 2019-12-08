#include "catch.hpp"

#include <iostream>
#include <set>

#include <cachew/lru_cache.hpp>

#include "common.hpp"

using namespace cachew;

TEST_CASE( "LRU iterator" )
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

TEST_CASE( "LRU cache size" )
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

TEMPLATE_TEST_CASE( "LRU ctors and assignment", "", int, float,
                    std::string ) // NOLINT
{
    const size_t data_len  = 1000;
    const size_t cache_len = data_len / 2;

    std::vector<TestType> buff;
    gen_test_seq( data_len, buff );

    lru_cache<int, TestType> cache( cache_len );

    auto m = buff.begin();
    std::advance( m, cache_len );
    std::set<TestType> expected( m, buff.end() );

    for( size_t i = 0; i < buff.size(); i++ )
    {
        cache.put( i, buff[i] );
    }

    SECTION( "ctors" )
    {
        lru_cache<int, TestType> cache_new( cache ); // NOLINT

        CHECK( to_set( cache_new ) == expected );
    }

    SECTION( "assignment" )
    {
        lru_cache<int, TestType> cache_new( cache_len );

        cache_new = cache;

        CHECK( to_set( cache_new ) == expected );
    }

    SECTION( "swap" )
    {
        lru_cache<int, TestType> cache_new( cache_len );

        std::swap( cache_new, cache );

        CHECK( cache.size() == 0 );
        CHECK( cache_new.size() == cache_len );
        CHECK( to_set( cache_new ) == expected );
    }
}
