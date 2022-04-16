#ifndef PINE_STD_VECTOR_H
#define PINE_STD_VECTOR_H

#include <initializer_list>

#include <pstd/new.h>
#include <pstd/move.h>
#include <pstd/math.h>
#include <pstd/algorithm.h>

namespace pstd {

template <typename T>
struct default_allocator {
    T* alloc(size_t size) const {
        return (T*)::operator new(sizeof(T) * size);
    }
    void free(T* ptr) const {
        ::operator delete(ptr);
    }

    template <typename... Args>
    void construct_at(T* ptr, Args&&... args) const {
        pstd::construct_at(ptr, pstd::forward<Args>(args)...);
    }

    void destruct_at(T* ptr) const {
        pstd::destruct_at(ptr);
    }
};

template <typename T, typename Allocator>
class vector_base {
  public:
    using value_type = T;

    using pointer = T*;

    using reference = T&;
    using const_reference = const T&;

    using iterator = T*;
    using const_iterator = const T*;

    ~vector_base() {
        clear();
    }

    vector_base() = default;

    explicit vector_base(size_t len) {
        resize(len);
    }
    vector_base(size_t len, const T& val) : vector_base(len) {
        pstd::fill(begin(), end(), val);
    }
    vector_base(std::initializer_list<T> list) : vector_base(pstd::size(list)) {
        pstd::copy(pstd::begin(list), pstd::end(list), begin());
    }
    template <typename It>
    vector_base(It first, It last) : vector_base(pstd::distance(first, last)) {
        pstd::copy(first, last, begin());
    }

    vector_base(const vector_base& rhs) : vector_base() {
        *this = rhs;
    }
    vector_base(vector_base&& rhs) : vector_base() {
        *this = pstd::move(rhs);
    }

    vector_base& operator=(const vector_base& rhs) {
        resize(pstd::size(rhs));
        pstd::copy(pstd::begin(rhs), pstd::end(rhs), begin());
        allocator = rhs.allocator;

        return *this;
    }
    vector_base& operator=(vector_base&& rhs) {
        pstd::swap(ptr, rhs.ptr);
        pstd::swap(len, rhs.len);
        pstd::swap(reserved, rhs.reserved);
        pstd::swap(allocator, rhs.allocator);
        return *this;
    }

    template <typename It>
    void assign(It first, It last) {
        resize(pstd::distance(first, last));
        pstd::copy(first, last, begin());
    }

    template <typename U = T>
    void push_back(U&& val) {
        reserve(size() + 1);
        allocator.construct_at(&(*end()), pstd::forward<U>(val));
        len += 1;
    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
        reserve(size() + 1);
        allocator.construct_at(&(*end()), T(pstd::forward<Args>(args)...));
        len += 1;
    }

    void pop_back() {
        resize(size() - 1);
    }

    template <typename U>
    iterator insert(iterator it, U&& val) {
        ptrdiff_t dist = it - begin();
        reserve(size() + 1);
        it = begin() + dist;

        auto i = end();
        for (; i > it; --i) {
            auto prev = i;
            --prev;
            pstd::swap(*prev, *i);
        }

        *it = pstd::forward<U>(val);
        ++len;

        return it;
    }

    void resize(size_t nlen) {
        // TODO #optimization#: resize() default-construct objects which is unnecessary in case
        // where we rewrite them right away
        reserve(nlen);
        for (size_t i = size(); i < nlen; ++i)
            allocator.construct_at(&ptr[i]);
        for (size_t i = nlen; i < size(); ++i)
            allocator.destruct_at(&ptr[i]);
        len = nlen;
    }

    void reserve(size_t nreserved) {
        nreserved = roundup2(nreserved);
        if (nreserved <= reserved)
            return;

        pointer nptr = allocator.alloc(nreserved);
        pstd::memcpy(nptr, ptr, size() * sizeof(T));

        ptr = nptr;
        reserved = nreserved;
    }

    void clear() {
        if (ptr) {
            for (size_t i = 0; i < size(); ++i)
                allocator.destruct_at(&ptr[i]);
            allocator.free(ptr);
        }
        ptr = nullptr;
        len = 0;
        reserved = 0;
    }

    reference operator[](size_t i) {
        return ptr[i];
    }
    const_reference operator[](size_t i) const {
        return ptr[i];
    }

    iterator begin() {
        return ptr;
    }
    iterator end() {
        return ptr + size();
    }
    const_iterator begin() const {
        return ptr;
    }
    const_iterator end() const {
        return ptr + size();
    }
    reference back() {
        return ptr[size() - 1];
    }
    const_reference back() const {
        return ptr[size() - 1];
    }

    size_t size() const {
        return len;
    }

    const T* data() const {
        return ptr;
    }

  protected:
    T* ptr = nullptr;
    size_t len = 0;
    size_t reserved = 0;

    Allocator allocator;
};

template <typename T>
using vector = vector_base<T, default_allocator<T>>;

}  // namespace pstd

#endif  // PINE_STD_VECTOR_H