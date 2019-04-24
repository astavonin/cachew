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

    template <class _Storage = lfu_map>
    class lfu_const_iterator
    {
    private:
        using difference_type   = std::ptrdiff_t;
        using value_type        = value_type;
        using pointer           = const value_type *;
        using reference         = const value_type &;
        using iterator_category = std::forward_iterator_tag;

        using actual_iterator_t = lfu_const_iterator<_Storage>;
        using parent_it_t       = typename _Storage::const_iterator;

        parent_it_t _it;

    public:
        lfu_const_iterator() = default;

        lfu_const_iterator( const actual_iterator_t &it )
            : _it( it._it )
        {
        }

        explicit lfu_const_iterator( const parent_it_t &it )
            : _it( it )
        {
        }

        bool operator==( const actual_iterator_t &it ) const
        {
            return it._it == _it;
        }

        bool operator!=( const actual_iterator_t &it ) const
        {
            return it._it != _it;
        }

        const actual_iterator_t &operator++()
        {
            ++_it;
            return *this;
        }

        const actual_iterator_t operator++( int )
        {
            actual_iterator_t res( *this );
            ++( *this );
            return res;
        }

        const value_type &operator*() const
        {
            return ( _it->second.second->second );
        }

        const value_type *operator->() const
        {
            return &( _it->second.second->second );
        }
    };

    using iterator = lfu_const_iterator<lfu_map>;

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
            return iterator{ _map.end() };
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
            node_location_pair &location = it->second;

            promote( location );
            location.second->second = std::forward<_PutT>( value );
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

    iterator begin() const noexcept
    {
        return iterator( _map.begin() );
    }

    iterator end() const noexcept
    {
        return iterator( _map.end() );
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
            _list.emplace_back( freq_node{cur_node.frequency + 1} );
            new_pos = std::prev( _list.end() );
        }
        freq_node &freq_node = *new_pos;
        freq_node.values.splice( freq_node.values.begin(),
                                 ( *location.first ).values, location.second );
        // location.second already should be updated
        location.first = new_pos;
    }

    void evict()
    {
        freq_node &freq_node = *( _list.begin() );
        auto           to_del    = std::prev( freq_node.values.end() );

        _map.erase( ( *to_del ).first );
        freq_node.values.erase( to_del );
    }

    freq_list _list;
    lfu_map   _map;
    size_t    _capacity;
};

} // namespace cachew

#endif // CACHEW_LFU_CACHE_HPP
