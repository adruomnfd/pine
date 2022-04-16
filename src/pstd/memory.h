#ifndef PINE_STD_MEMORY_H
#define PINE_STD_MEMORY_H

#include <pstd/new.h>
#include <pstd/move.h>
#include <pstd/stdint.h>
#include <pstd/archive.h>
#include <pstd/type_traits.h>

namespace pstd {

inline constexpr void memcpy(void* dst, const void* src, size_t size) {
    auto csrc = (const char*)src;
    auto cdst = (char*)dst;
    for (size_t i = 0; i != size; ++i)
        cdst[i] = csrc[i];
}

inline constexpr void memset(void* dst, char val, size_t size) {
    auto cdst = (char*)dst;
    for (size_t i = 0; i != size; ++i)
        cdst[i] = val;
}

template <typename T, typename... Args>
inline void construct_at(T* ptr, Args&&... args) {
    ::new (ptr) T(pstd::forward<Args>(args)...);
}

template <typename T>
inline void destruct_at(T* ptr) {
    ptr->T::~T();
}

template <typename T>
struct default_delete {
    default_delete() = default;

    template <typename U>
    default_delete(const default_delete<U>&) noexcept {
    }

    void operator()(remove_extent_t<T>* ptr) const {
        if constexpr (is_array_v<T>)
            delete[] ptr;
        else
            delete ptr;
    }
};

// unique_ptr
template <typename T, typename Deleter = default_delete<T>>
class unique_ptr {
  public:
    using pointer = remove_extent_t<T>*;
    using reference = remove_extent_t<T>&;

    template <typename U, typename UDeleter>
    friend class unique_ptr;

    ~unique_ptr() {
        if (ptr != pointer())
            deleter(ptr);
    }

    unique_ptr() = default;
    unique_ptr(nullptr_t) {
    }
    explicit unique_ptr(pointer ptr, Deleter deleter = {}) : ptr(ptr), deleter(deleter) {
    }

    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;

    unique_ptr(unique_ptr&& rhs) : unique_ptr() {
        *this = pstd::move(rhs);
    }
    unique_ptr& operator=(unique_ptr&& rhs) {
        swap(rhs);
        return *this;
    }

    template <typename U, typename UDeleter, typename = enable_if_t<is_convertible_v<U*, T*>>>
    unique_ptr(unique_ptr<U, UDeleter>&& rhs) : unique_ptr() {
        *this = pstd::move(rhs);
    }
    template <typename U, typename UDeleter, typename = enable_if_t<is_convertible_v<U*, T*>>>
    unique_ptr& operator=(unique_ptr<U, UDeleter>&& rhs) {
        swap(rhs);
        return *this;
    }

    reference operator*() const {
        return *ptr;
    }
    pointer operator->() const {
        return ptr;
    }
    reference operator[](size_t i) const {
        static_assert(is_array_v<T>,
                      "operator[] can only be used when the underlying type is array");
        return ptr[i];
    }

    explicit operator bool() const {
        return get() != pointer();
    }

    pointer get() const {
        return ptr;
    }

    pointer release() {
        pointer p = ptr;
        ptr = {};
        return p;
    }

    void reset(pointer p = {}) {
        pstd::swap(ptr, p);
        if (p != pointer())
            deleter(p);
    }

    template <typename U, typename UDeleter>
    friend bool operator==(const unique_ptr& lhs, const unique_ptr<U, UDeleter>& rhs) {
        return lhs.get() == rhs.get();
    }
    template <typename U, typename UDeleter>
    friend bool operator!=(const unique_ptr& lhs, const unique_ptr<U, UDeleter>& rhs) {
        return lhs.get() != rhs.get();
    }
    template <typename U, typename UDeleter>
    friend bool operator>(const unique_ptr& lhs, const unique_ptr<U, UDeleter>& rhs) {
        return lhs.get() > rhs.get();
    }
    template <typename U, typename UDeleter>
    friend bool operator<(const unique_ptr& lhs, const unique_ptr<U, UDeleter>& rhs) {
        return lhs.get() < rhs.get();
    }

    PSTD_ARCHIVE(ptr)

  private:
    template <typename U, typename UDeleter>
    void swap(unique_ptr<U, UDeleter>& rhs) {
        pstd::swap(ptr, (pointer&)rhs.ptr);
        pstd::swap(deleter, (Deleter&)rhs.deleter);
    }

    pointer ptr = {};
    Deleter deleter = {};
};

template <typename T, typename... Args>
inline unique_ptr<T> make_unique(Args&&... args) {
    return unique_ptr<T>(new T(pstd::forward<Args>(args)...));
}

// shared_ptr
template <typename T, typename Deleter = default_delete<T>>
class shared_ptr {
  public:
    using pointer = remove_extent_t<T>*;
    using reference = remove_extent_t<T>&;

    template <typename U, typename UDeleter>
    friend class shared_ptr;

    ~shared_ptr() {
        decrement();
    }

    shared_ptr() = default;

    shared_ptr(nullptr_t) {
    }
    explicit shared_ptr(pointer ptr, Deleter deleter = {})
        : ptr(ptr), deleter(deleter), refcount(new size_t(1)) {
    }

    shared_ptr(const shared_ptr& rhs) : shared_ptr() {
        *this = rhs;
    }
    shared_ptr(shared_ptr&& rhs) : shared_ptr() {
        *this = pstd::move(rhs);
    }
    shared_ptr& operator=(const shared_ptr& rhs) {
        copy(rhs);
        return *this;
    }
    shared_ptr& operator=(shared_ptr&& rhs) {
        swap(rhs);
        return *this;
    }

    template <typename U, typename UDeleter, typename = enable_if_t<is_convertible_v<U*, T*>>>
    shared_ptr(const shared_ptr<U, UDeleter>& rhs) : shared_ptr() {
        *this = rhs;
    }
    template <typename U, typename UDeleter, typename = enable_if_t<is_convertible_v<U*, T*>>>
    shared_ptr(shared_ptr<U, UDeleter>&& rhs) : shared_ptr() {
        *this = pstd::move(rhs);
    }
    template <typename U, typename UDeleter, typename = enable_if_t<is_convertible_v<U*, T*>>>
    shared_ptr& operator=(const shared_ptr<U, UDeleter>& rhs) {
        copy(rhs);
        return *this;
    }
    template <typename U, typename UDeleter, typename = enable_if_t<is_convertible_v<U*, T*>>>
    shared_ptr& operator=(shared_ptr<U, UDeleter>&& rhs) {
        swap(rhs);
        return *this;
    }

    reference operator*() const {
        return *ptr;
    }
    pointer operator->() const {
        return ptr;
    }
    reference operator[](size_t i) const {
        static_assert(is_array_v<T>,
                      "operator[] can only be used when the underlying type is array");
        return ptr[i];
    }

    explicit operator bool() const {
        return get() != pointer();
    }

    pointer get() const {
        return ptr;
    }

    void reset(pointer p = {}) {
        decrement();

        ptr = p;
        refcount = new size_t(1);
    }

    template <typename U, typename UDeleter>
    friend bool operator==(const shared_ptr& lhs, const shared_ptr<U, UDeleter>& rhs) {
        return lhs.get() == rhs.get();
    }
    template <typename U, typename UDeleter>
    friend bool operator!=(const shared_ptr& lhs, const shared_ptr<U, UDeleter>& rhs) {
        return lhs.get() != rhs.get();
    }
    template <typename U, typename UDeleter>
    friend bool operator>(const shared_ptr& lhs, const shared_ptr<U, UDeleter>& rhs) {
        return lhs.get() > rhs.get();
    }
    template <typename U, typename UDeleter>
    friend bool operator<(const shared_ptr& lhs, const shared_ptr<U, UDeleter>& rhs) {
        return lhs.get() < rhs.get();
    }

    PSTD_ARCHIVE(ptr)

  private:
    template <typename U, typename UDeleter>
    void copy(const shared_ptr<U, UDeleter>& rhs) {
        decrement();

        ptr = rhs.ptr;
        deleter = rhs.deleter;
        refcount = rhs.refcount;

        if (refcount)
            ++(*refcount);
    }

    template <typename U, typename UDeleter>
    void swap(shared_ptr<U, UDeleter>& rhs) {
        pstd::swap(ptr, (pointer&)rhs.ptr);
        pstd::swap(deleter, (Deleter&)rhs.deleter);
        pstd::swap(refcount, rhs.refcount);
    }

    void decrement() {
        if (ptr != pointer()) {
            if (--(*refcount) == 0) {
                deleter(ptr);
                delete refcount;
            }
        }
    }

    pointer ptr = {};
    Deleter deleter = {};
    size_t* refcount = nullptr;
};
template <typename T, typename... Args>
inline shared_ptr<T> make_shared(Args&&... args) {
    return shared_ptr<T>(new T(pstd::forward<Args>(args)...));
}

}  // namespace pstd

#endif  // PINE_STD_MEMORY_H