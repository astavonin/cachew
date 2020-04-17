#ifndef CACHEW_CONCURRENT_CACHE_HPP
#define CACHEW_CONCURRENT_CACHE_HPP

#include "cache_iterator.hpp"

#include <algorithm>
#include <atomic>
#include <shared_mutex>
#include <thread>
#include <unordered_map>

namespace cachew
{

template <class key_type>
class list
{
public:
    struct node
    {
        node()
            : _prev( OUT_OF_LIST_NODE )
            , _next( nullptr )
        {
        }

        explicit node( const key_type &key )
            : _key( key )
            , _prev( OUT_OF_LIST_NODE )
            , _next( nullptr )
        {
        }

        key_type _key;
        node *   _prev;
        node *   _next;

        bool is_valid() const
        {
            return _prev != OUT_OF_LIST_NODE;
        }
    };

    inline void unlink( node *n )
    {
        assert( _head != n );

        node *prev  = n->_prev;
        node *next  = n->_next;
        prev->_next = next;
        next->_prev = prev;
        n->_prev    = OUT_OF_LIST_NODE;
    }

    void move_front( node *n )
    {
        assert( _head != n );

        n->_prev     = nullptr;
        n->_next     = _head;
        _head->_prev = n;
        _head        = n;
    }

    static node *const OUT_OF_LIST_NODE;

    node *     _head;
    node *     _tail;
    std::mutex _list_mutex;
};

template <class _Key, class _Tp>
class concurrent_cache
{
public:
    using key_type   = _Key;
    using value_type = _Tp;

    using kv_pair = std::pair<key_type, value_type>;

    using hash_fn   = std::hash<key_type>;
    using conc_list = list<value_type>;
    using node_ptr  = typename list<key_type>::node *;

    using vi_pair = std::pair<value_type, node_ptr>;

private:
    class bucket
    {
        using storage = std::unordered_map<key_type, node_ptr>;

    public:
        bucket()  = default;
        ~bucket() = default;

        bool find( const key_type &key )
        {
            std::shared_lock l{ _bucket_mutex };

            return _map.find( key ) != _map.end();
        }

        std::optional<vi_pair> get( const key_type &key )
        {
            std::shared_lock l{ _bucket_mutex };

            auto it = _map.find( key );
            if( it != _map.end() )
            {
                return it->second;
            }

            return std::nullopt;
        }

        node_ptr put( const key_type &key, vi_pair value )
        {
            std::shared_lock l{ _bucket_mutex };

            node_ptr old_node = nullptr;

            auto it = _map.find( key );
            if( it == _map.end() || it->second != value )
            {
                l.unlock();
                l.lock();
                auto it = _map.find( key );

                if( it == _map.end() )
                {
                    _map.emplace( key, std::move( value ) );
                    // TODO: emplace may throw an exception
                }
                else if( it->second != value )
                {
                    old_node  = _map[key].second;
                    _map[key] = std::move( value );
                }
            }
            return old_node;
        }

        void remove( const key_type &key )
        {
            std::shared_lock l{ _bucket_mutex };
        }

    private:
        storage                      _map;
        std::shared_mutex            _bucket_mutex;
        typename conc_list::iterator _no_key;
    };

public:
    friend bool operator!=( const concurrent_cache &lhs,
                            const concurrent_cache &rhs )
    {
        return !( rhs == lhs );
    }

    explicit concurrent_cache( size_t capacity )
        : _capacity( capacity )
        , _buckets_count( std::thread::hardware_concurrency() )
    {
        _buckets.reserve( _buckets_count );
        for( int i = 0; i < _buckets_count; ++i )
        {
            _buckets.emplace_back( bucket( _list.end() ) );
        }
    }

    std::optional<value_type> get( const key_type &key )
    {
        size_t  bucket_nr = hash_fn()( key ) % ( _buckets_count - 1 );
        bucket &bucket    = _buckets[bucket_nr];

        auto res = bucket.get( key );

        if( res )
        {
            if( _list_mutex.try_lock() )
            {
                _list.splice( _list.begin(), _list, ( *res ).second );
            }
        }

        return !res ? std::nullopt : ( *res ).first;
    }

    template <class _PutT>
    void put( const key_type &key, _PutT &&value )
    {
        size_t  bucket_nr = hash_fn()( key ) % ( _buckets_count - 1 );
        bucket &bucket    = _buckets[bucket_nr];

        typename conc_list::node *node =
            new typename list<key_type>::node{ key };
        auto old_node =
            bucket.put( key, std::make_pair( std::forward( value ), node ) );

        {
            std::unique_lock ll( _list_mutex );

            // updated node, we should remove old value
            if( old_node != nullptr )
            {
                _list.unlink( old_node );
                delete old_node;
            }

            _list.move_front( node );
            // TODO: eviction
        }
    }

    size_t capacity() const noexcept
    {
        return _capacity;
    }

    size_t size() const noexcept
    {
        return _size.load();
    }

private:
    conc_list           _list;
    std::shared_mutex   _list_mutex;
    size_t              _capacity;
    size_t              _buckets_count;
    std::vector<bucket> _buckets;
    std::atomic<size_t> _size;
};

} // namespace cachew

#endif // CACHEW_CONCURRENT_CACHE_HPP
