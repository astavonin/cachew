#ifndef CACHEW_LRU_CACHE_HPP
#define CACHEW_LRU_CACHE_HPP

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

    template <class _Storage = std::list<kv_pair>>
    class lru_iterator
    {
    private:
        using it_type        = lru_iterator<_Storage>;
        using parent_it_type = typename _Storage::const_iterator;

        parent_it_type _it;

    public:
        lru_iterator() = default;

        lru_iterator( const it_type &it )
            : _it( it._it )
        {
        }

        explicit lru_iterator( const parent_it_type &it )
            : _it( it )
        {
        }

        bool operator==( const it_type &it ) const
        {
            return it._it == _it;
        }

        bool operator!=( const it_type &it ) const
        {
            return it._it != _it;
        }

        const it_type &operator++()
        {
            ++_it;
            return *this;
        }

        const it_type operator++( int )
        {
            it_type res( *this );
            ++( *this );
            return res;
        }

        const value_type &operator*() const
        {
            return ( _it->second );
        }

        const value_type *operator->() const
        {
            return &( _it->second );
        }
    };

    using lru_list = std::list<std::pair<key_type, value_type>>;
    using lru_map  = std::unordered_map<key_type, typename lru_list::iterator>;

    using iterator = lru_iterator<lru_list>;

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

    template <class _PutK, class _PutT>
    void put( _PutK &&key, _PutT &&value )
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
        _list.emplace_front( std::forward<key_type>( key ),
                             std::forward<value_type>( value ) );
        _map.emplace( std::forward<key_type>( key ), _list.begin() );
    }

    size_t capacity() const
    {
        return _capacity;
    }

    size_t size() const
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
