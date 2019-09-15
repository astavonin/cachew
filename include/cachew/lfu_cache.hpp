#ifndef CACHEW_LFU_CACHE_HPP
#define CACHEW_LFU_CACHE_HPP

// http://dhruvbird.com/lfu.pdf

#include "cache_iterator.hpp"

#include <algorithm>
#include <list>
#include <unordered_map>

namespace cachew
{

template <class _Key, class _Tp>
class lfu_cache
{
public:
    using key_type   = _Key;
    using value_type = _Tp;

    using kv_pair = std::pair<key_type, value_type>;

    struct freq_node
    {
        using values_list = std::list<kv_pair>;

        size_t      frequency;
        values_list values;
    };

    using freq_list = std::list<freq_node>;
    using node_location_pair =
        std::pair<typename freq_list::iterator,
                  typename freq_node::values_list::iterator>;
    using lfu_map = std::unordered_map<key_type, node_location_pair>;

private:
    struct accessor
    {
        using const_iterator = typename lfu_map::const_iterator;

        inline explicit accessor( const typename lfu_map ::const_iterator &it )
            : _it( it )
        {
        }
        inline const value_type &ref() const
        {
            return ( _it->second.second->second );
        }
        inline const value_type *ptr() const
        {
            return &( _it->second.second->second );
        }

        const typename lfu_map::const_iterator &_it;
    };

public:
    using iterator = cache_const_iterator<accessor, value_type>;

    friend bool operator!=( const lfu_cache &lhs, const lfu_cache &rhs )
    {
        return !( rhs == lhs );
    }

    explicit lfu_cache( size_t capacity ) noexcept
        : _capacity( capacity )
    {
    }

    iterator get( const key_type &key )
    {
        auto it = _map.find( key );
        if( it == _map.end() )
        {
            return iterator{_map.end()};
        }
        node_location_pair &location = it->second;

        promote( location );

        return iterator( it );
    }

    template <class _PutT>
    void put( const key_type &key, _PutT &&value )
    {
        auto it = _map.find( key );
        if( it != _map.end() )
        {
            node_location_pair &cur_loc = it->second;

            auto new_loc           = promote( cur_loc );
            new_loc.second->second = std::forward<_PutT>( value );

            // it's safe to update internal state as no exceptions are expected
            // after this line
            std::swap( cur_loc, new_loc );
        }
        else
        {
            if( _map.size() == _capacity && _capacity > 0 )
            {
                evict();
            }
            auto pos = _list.begin();
            if( pos == _list.end() )
            {
                _list.emplace_back( freq_node{1} );
                pos = _list.begin();
            }
            freq_node &freq_node = *pos;
            freq_node.values.emplace_front(
                std::make_pair( key, std::forward<_PutT>( value ) ) );
            try
            {
                _map.emplace( key,
                              std::make_pair( pos, freq_node.values.begin() ) );
            }
            catch( ... )
            {
                // already emplaced `values_list` will not be removed as it
                // doesn't affect cache consistency
                freq_node.values.pop_front();
                throw;
            }
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
        return iterator( _map.begin() );
    }

    iterator end() const noexcept
    {
        return iterator( _map.end() );
    }

private:
    node_location_pair promote( node_location_pair location )
    {
        auto  new_pos  = std::next( location.first );
        auto &cur_node = *( location.first );
        if( new_pos == _list.end() )
        {
            _list.emplace_back( freq_node{cur_node.frequency + 1} );
            new_pos = std::prev( _list.end() );
        }
        freq_node &freq_node = *new_pos;
        freq_node.values.splice( freq_node.values.begin(),
                                 ( *location.first ).values, location.second );
        // location.second already should be updated
        location.first = new_pos;

        return location;
    }

    void evict()
    {
        freq_node &freq_node = *( _list.begin() );
        auto       to_del    = std::prev( freq_node.values.end() );

        _map.erase( ( *to_del ).first );
        freq_node.values.erase( to_del );
    }

    freq_list _list;
    lfu_map   _map;
    size_t    _capacity;
};

} // namespace cachew

#endif // CACHEW_LFU_CACHE_HPP