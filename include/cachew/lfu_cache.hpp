#ifndef CACHEW_LFU_CACHE_HPP
#define CACHEW_LFU_CACHE_HPP

// http://dhruvbird.com/lfu.pdf

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

    struct lfu_freq_node
    {
        using values_list = std::list<kv_pair>;

        size_t      frequency;
        values_list values;
    };

    using freq_list = std::list<lfu_freq_node>;
    using node_location_pair =
        std::pair<typename freq_list::iterator,
                  typename lfu_freq_node::values_list::iterator>;
    using lfu_map = std::unordered_map<key_type, node_location_pair>;

    explicit lfu_cache( size_t capacity ) noexcept
        : _capacity( capacity )
    {
    }

    value_type get( const key_type &key )
    {
        auto it = _map.find( key );
        if( it == _map.end() )
        {
            return value_type{};
        }
        node_location_pair &location = it->second;

        promote( location );

        return location.second->second;
    }

    template <class _PutT>
    void put( const key_type &key, _PutT &&value )
    {
        auto it = _map.find( key );
        if( it != _map.end() )
        {
            node_location_pair &location = it->second;

            promote( location );
            location.second->second = std::forward<_PutT>( value );
        }
        else
        {
            if( _map.size() == _capacity && _capacity > 0 )
            {
                lfu_freq_node &freq_node = *( _list.begin() );
                auto           to_del    = std::prev( freq_node.values.end() );

                _map.erase( ( *to_del ).first );
                freq_node.values.erase( to_del );
            }
            auto pos = _list.begin();
            if( pos == _list.end() )
            {
                _list.emplace_back( lfu_freq_node{1} );
                pos = _list.begin();
            }
            lfu_freq_node &freq_node = *pos;
            freq_node.values.emplace_front(
                std::make_pair( key, std::forward<_PutT>( value ) ) );
            _map.emplace( key,
                          std::make_pair( pos, freq_node.values.begin() ) );
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

    void dump_debug( std::ostream &stream )
    {
        stream << "[";
        for( const auto &node : _list )
        {
            stream << node.frequency << ": {";
            for( auto const &val : node.values )
            {
                stream << val.first << " ";
            }

            stream << "}, ";
        }
        stream << "]" << std::endl;
    }

private:
    void promote( node_location_pair &location )
    {
        auto  new_pos  = std::next( location.first );
        auto &cur_node = *( location.first );
        if( new_pos == _list.end() )
        {
            _list.emplace_back( lfu_freq_node{cur_node.frequency + 1} );
            new_pos = std::prev( _list.end() );
        }
        lfu_freq_node &freq_node = *new_pos;
        freq_node.values.splice( freq_node.values.begin(),
                                 ( *location.first ).values, location.second );
        // location.second already should be updated
        location.first = new_pos;
    }

    freq_list _list;
    lfu_map   _map;
    size_t    _capacity;
};

} // namespace cachew

#endif // CACHEW_LFU_CACHE_HPP
