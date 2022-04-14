#ifndef PINE_STD_VECTOR_H
#define PINE_STD_VECTOR_H

#include <initializer_list>

#include <pstd/new.h>
#include <pstd/move.h>
#include <pstd/algorithm.h>

namespace pstd {

template <typename T>
struct default_alloc {
    T* alloc(size_t size) const {
        return new T[size]();
    }
    void free(T* ptr) const {
        delete[] ptr;
    }
};

template <typename T, typename Alloc>
class vector_base : public Alloc {
  public:
    using pointer = T*;
    using reference = T&;
    using const_reference = const T&;

    using iterator = T*;
    using const_iterator = const T*;

    using Alloc::alloc;
    using Alloc::free;

    vector_base() = default;
    ~vector_base() {
        clear();
    }

    explicit vector_base(size_t len) : len(len), reserved(len) {
        ptr = alloc(len);
    }
    vector_base(size_t len, T val) : vector_base(len) {
        fill(begin(), end(), val);
    }
    vector_base(std::initializer_list<T> list) : vector_base(list.size()) {
        copy(list.begin(), list.end(), begin());
    }

    vector_base(const vector_base& rhs) : vector_base() {
        *this = rhs;
    }
    vector_base(vector_base&& rhs) : vector_base() {
        *this = move(rhs);
    }

    vector_base& operator=(const vector_base& rhs) {
        clear();
        ptr = alloc(rhs.len);
        len = rhs.len;
        reserved = rhs.reserved;
        copy(rhs.begin(), rhs.end(), begin());

        return *this;
    }
    vector_base& operator=(vector_base&& rhs) {
        swap(ptr, rhs.ptr);
        swap(len, rhs.len);
        swap(reserved, rhs.reserved);
        return *this;
    }

    template <typename U>
    void push_back(U&& val) {
        reserve(size() + 1);
        *end() = forward<U>(val);
        len += 1;
    }

    template <typename... Args>
    void emplace_back(Args&&... args) {
        reserve(size() + 1);
        new (&ptr[size()]) T(forward<Args>(args)...);
        len += 1;
    }

    template <typename U>
    iterator insert(iterator it, U&& val) {
        ptrdiff_t dist = it - begin();
        resize(size() + 1);
        it = begin() + dist;

        auto i = end();
        --i;
        for (; i > it; --i) {
            auto prev = i;
            --prev;
            swap(*prev, *i);
        }

        *it = forward<U>(val);

        return it;
    }

    void resize(size_t nlen) {
        reserve(nlen);
        len = nlen;
    }

    void reserve(size_t nreserved) {
        if (nreserved <= reserved)
            return;
        pointer nptr = alloc(roundup2(nreserved));
        copy(begin(), end(), nptr);
        reset(nptr, size(), nreserved);
    }

    void clear() {
        reset(pointer(), 0, 0);
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
    void reset(pointer nptr, size_t nlen, size_t nreserved) {
        if (ptr)
            free(ptr);
        ptr = nptr;
        len = nlen;
        reserved = nreserved;
    }

    T* ptr = nullptr;
    size_t len = 0;
    size_t reserved = 0;
};

template <typename T>
using vector = vector_base<T, default_alloc<T>>;

}  // namespace pstd

#endif  // PINE_STD_VECTOR_H