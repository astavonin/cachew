#ifndef CACHEW_COMMON_HPP
#define CACHEW_COMMON_HPP

#include <vector>
#include <set>

template <class _Cont, class V = typename _Cont::value_type>
std::set<V> to_set( const _Cont &cache )
{
    std::set<V> res;
    for( auto val : cache )
    {
        res.emplace( val );
    }
    return res;
}

template <typename T>
void gen_test_seq( size_t len, std::vector<T> &res )
{
    res.resize( len );
    for( int i = 0; i < len; ++i )
    {
        res[i] = i * 10;
    }
}

template <>
void gen_test_seq( size_t len, std::vector<std::string> &res );

#endif // CACHEW_COMMON_HPP
