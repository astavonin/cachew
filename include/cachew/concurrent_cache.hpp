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
        static node *const OUT_OF_LIST_NODE;

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

        [[nodiscard]] inline bool is_valid() const
        {
            return _prev != OUT_OF_LIST_NODE;
        }

        key_type _key;
        node *   _prev;
        node *   _next;
    };

    inline void unlink( node *n )
    {
        assert( _head != n );

        node *prev  = n->_prev;
        node *next  = n->_next;
        prev->_next = next;
        if( next )
        {
            next->_prev = prev;
        }
        n->_prev = node::OUT_OF_LIST_NODE;
    }

    inline void move_front( node *n )
    {
        assert( _head != n );

        n->_prev     = nullptr;
        n->_next     = _head;
        _head->_prev = n;
        _head        = n;
    }

    inline node *pop_back()
    {
        node *to_remove = _tail;
        unlink( _tail );
        return to_remove;
    }

    inline node *back()
    {
        return _tail;
    }

    node *_head;
    node *_tail;
};

template <class key_type>
typename list<key_type>::node *const
    list<key_type>::node::OUT_OF_LIST_NODE = (node *)-1;

template <class Key, class Tp>
class concurrent_cache
{
public:
    using key_type   = Key;
    using value_type = Tp;

    using kv_pair = std::pair<key_type, value_type>;

    using hash_fn   = std::hash<key_type>;
    using conc_list = list<value_type>;
    using node_ptr  = typename list<key_type>::node *;

    using vi_pair = std::pair<value_type, node_ptr>;

private:
    class bucket
    {
        using storage = std::unordered_map<key_type, vi_pair>;

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
                it = _map.find( key );

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
        storage           _map;
        std::shared_mutex _bucket_mutex;
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
        , _size( 0 )
    {
        _buckets.reserve( _buckets_count );
        for( size_t i = 0; i < _buckets_count; ++i )
        {
            _buckets.emplace_back( new bucket() );
        }
    }

    std::optional<value_type> get( const key_type &key )
    {
        bucket *bucket = find_bucket( key );

        auto res = bucket->get( key );

        if( res )
        {
            if( _list_mutex.try_lock() && ( *res ).second->is_valid() )
            {
                _list.move_front( ( *res ).second );
            }
        }

        return !res ? std::nullopt
                    : std::optional<key_type>( std::in_place, ( *res ).first );
    }

    inline bucket *find_bucket( const key_type &key )
    {
        size_t bucket_nr = hash_fn()( key ) % ( _buckets_count - 1 );
        return _buckets[bucket_nr];
    }

    void evict()
    {
        std::unique_lock ll( _list_mutex );
        auto             to_evict = _list.pop_back();
        ll.unlock();

        if( to_evict )
        {
            key_type key    = to_evict->_key;
            bucket * bucket = find_bucket( key );
            bucket->remove( key );
        }
    }

    template <class PutT>
    void put( const key_type &key, PutT &&value )
    {
        bucket *bucket = find_bucket( key );

        node_ptr new_node = new typename list<key_type>::node{ key };
        node_ptr old_node =
            bucket->put( key, std::make_pair( value, new_node ) );
        // key, std::make_pair( std::forward( value ), new_node ) );

        size_t cur_size     = _size.load();
        bool   size_changed = true;
        if( cur_size >= _capacity &&
            old_node == nullptr ) // if old_node == nullptr this is insertation,
                                  // and storage size will growth.
        {
            evict();
            size_changed = false;
        }

        {
            std::unique_lock ll( _list_mutex );

            // updated node, we should remove old value
            if( old_node != nullptr )
            {
                _list.unlink( old_node );
                delete old_node;
            }

            _list.move_front( new_node );
        }
        if( size_changed )
        {
            _size++;
        }
    }

    [[nodiscard]] size_t capacity() const noexcept
    {
        return _capacity;
    }

    [[nodiscard]] size_t size() const noexcept
    {
        return _size.load();
    }

private:
    conc_list             _list;
    std::shared_mutex     _list_mutex;
    size_t                _capacity;
    size_t                _buckets_count;
    std::vector<bucket *> _buckets;
    std::atomic<size_t>   _size;
};
} // namespace cachew

#endif // CACHEW_CONCURRENT_CACHE_HPP
