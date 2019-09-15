#ifndef CACHEW_CACHE_ITERATOR_HPP
#define CACHEW_CACHE_ITERATOR_HPP

namespace cachew
{
template <typename accessor_t, typename _Storage, typename value_type>
class cache_const_iterator
{
private:
    using difference_type   = std::ptrdiff_t;
    using pointer           = const value_type *;
    using reference         = const value_type &;
    using iterator_category = std::forward_iterator_tag;

    using actual_iterator_t =
        cache_const_iterator<accessor_t, _Storage, value_type>;
    using parent_it_t = typename _Storage::const_iterator;

    parent_it_t _it;

public:
    cache_const_iterator() = default;

    cache_const_iterator( const actual_iterator_t &it )
        : _it( it._it )
    {
    }

    explicit cache_const_iterator( const parent_it_t &it )
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
        return accessor_t( _it ).ref();
    }

    const value_type *operator->() const
    {
        return accessor_t( _it ).ptr();
    }
};
} // namespace cachew

#endif // CACHEW_CACHE_ITERATOR_HPP
