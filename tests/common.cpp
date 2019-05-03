#include "common.hpp"

#include <random>

template <>
void gen_test_seq( size_t len, std::vector<std::string> &res )
{
    static const char alphanum[] = "0123456789"
                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz";
    std::random_device              rd;
    std::mt19937                    gen( rd() );
    std::uniform_int_distribution<> dis( 0, sizeof( alphanum ) - 1 );

    res.resize( len );

    for( int i = 0; i < len; ++i )
    {
        std::string buf( 10, '\0' );
        for( int j = 0; j < 10; ++j )
        {
            buf[j] = alphanum[dis( gen )];
        }
        res[i] = std::move( buf );
    }
}
