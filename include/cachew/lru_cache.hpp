#ifndef CACHEW_LRU_CACHE_HPP
#define CACHEW_LRU_CACHE_HPP

#include "cache_iterator.hpp"

#include <algorithm>
#include <list>
#include <unordered_map>

namespace cachew
{

template <class _Key, class _Tp>
class lru_cache
{
public:
    using key_type   = _Key;
    using value_type = _Tp;

    using kv_pair = std::pair<key_type, value_type>;

    using lru_list = std::list<std::pair<key_type, value_type>>;
    using lru_map  = std::unordered_map<key_type, typename lru_list::iterator>;

private:
    struct accessor
    {
        inline explicit accessor( const typename lru_list::const_iterator &it )
            : _it( it )
        {
        }
        inline const value_type &ref() const
        {
            return _it->second;
        }
        inline const value_type *ptr() const
        {
            return &( _it->second );
        }

        const typename lru_list::const_iterator &_it;
    };

public:
    using iterator = cache_const_iterator<accessor, lru_list, value_type>;

    friend bool operator!=( const lru_cache &lhs, const lru_cache &rhs )
    {
        return !( rhs == lhs );
    }

    explicit lru_cache( size_t capacity ) noexcept
        : _capacity( capacity )
    {
    }

    iterator get( const key_type &key )
    {
        auto it = _map.find( key );
        if( it == _map.end() )
        {
            return iterator( _list.end() );
        }
        _list.splice( _list.begin(), _list, it->second );

        return iterator( it->second );
    }

    template <class _PutT>
    void put( const key_type &key, _PutT &&value )
    {
        auto it = _map.find( key );
        if( it != _map.end() )
        {
            _list.splice( _list.begin(), _list, it->second );
            it->second->second = value;
        }
        if( _map.size() == _capacity )
        {
            auto to_del = _list.back().first;
            _list.pop_back();
            _map.erase( to_del );
        }
        _list.emplace_front( key, std::forward<_PutT>( value ) );
        try
        {
            _map.emplace( key, _list.begin() );
        }
        catch( ... )
        {
            _list.pop_front();
            throw;
        }
    }

    size_t capacity() const noexcept
    {
        return _capacity;
    }

    size_t size() const noexcept
    {
        return _map.size();
    }

    iterator begin() const noexcept
    {
        return iterator( _list.begin() );
    }

    iterator end() const noexcept
    {
        return iterator( _list.end() );
    }

private:
    lru_list _list;
    lru_map  _map;
    size_t   _capacity;
};

} // namespace cachew

#endif // CACHEW_LRU_CACHE_HPP
