#pragma once

// std::allocator compatible using plain malloc to avoid operator new
template <typename T>
struct MallocAllocator
{
    typedef std::size_t    size_type;
    typedef std::ptrdiff_t difference_type;
    typedef T*             pointer;
    typedef const T*       const_pointer;
    typedef T&             reference;
    typedef const T&       const_reference;
    typedef T              value_type;

    pointer allocate(size_type n, const void* = 0) {
        return static_cast<pointer>(malloc(sizeof(value_type) * n));
    }

    void deallocate(pointer ptr, size_type) { free(ptr); }

    // boilerplate follows
    MallocAllocator() {}

    MallocAllocator(const MallocAllocator&) {}

    template <typename Other>
    MallocAllocator(const MallocAllocator<Other>&) {}

    MallocAllocator& operator=(const MallocAllocator&) { return *this; }

    template <class Other>
    MallocAllocator& operator=(const MallocAllocator<Other>&) {
        return *this;
    }

    template <typename Other>
    struct rebind
    { typedef MallocAllocator<Other> other; };

    size_type max_size() const throw() {
        // ugly size_t maximum but don't want to drag <numeric_limits> just because of this
        return std::size_t(-1) / sizeof(T);
    }

    pointer address(reference ref) const { return &ref; }

    const_pointer address(const_reference ref) const { return &ref; }

    void construct(pointer ptr, const value_type& val) { ::new(ptr) value_type(val); }

    void destroy(pointer ptr) { ptr->~value_type(); }
};

template <typename T, typename U>
inline bool operator==(const MallocAllocator<T>&, const MallocAllocator<U>&) {
    return true;
}

template <typename T, typename U>
inline bool operator!=(const MallocAllocator<T>& a, const MallocAllocator<U>& b) {
    return !(a == b);
}
