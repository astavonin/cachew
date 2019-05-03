#include "catch.hpp"

#include <iostream>
#include <set>

#include <cachew/lfu_cache.hpp>

#include "common.hpp"

using namespace cachew;

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

TEMPLATE_TEST_CASE( "LFU ctors and assignment", "", int, double, std::string )
{
    const size_t data_len  = 1000;
    const size_t cache_len = data_len / 2;

    std::vector<TestType> buff;
    gen_test_seq( data_len, buff );

    lfu_cache<int, TestType> cache( cache_len );

    auto m = buff.begin();
    std::advance( m, cache_len );
    std::set<TestType> expected( m, buff.end() );

    for( int i = 0; i < data_len; i++ )
    {
        cache.put( i, buff[i] );
    }

    SECTION( "ctors" )
    {
        lfu_cache<int, TestType> cache_new( cache );

        CHECK( to_set( cache_new ) == expected );
    }

    SECTION( "assignment" )
    {
        lfu_cache<int, TestType> cache_new( cache_len );

        cache_new = cache;

        CHECK( to_set( cache_new ) == expected );
    }

    SECTION( "swap" )
    {
        lfu_cache<int, TestType> cache_new( cache_len );

        std::swap( cache_new, cache );

        CHECK( cache.size() == 0 );
        CHECK( cache_new.size() == cache_len );
        CHECK( to_set( cache_new ) == expected );
    }
}
