#ifndef PINE_STD_OPTIONAL_H
#define PINE_STD_OPTIONAL_H

#include <pstd/move.h>

namespace pstd {

struct nullopt {};

template <typename T>
struct optional {
    optional() = default;
    optional(nullopt){};

    template <typename U>
    optional(U&& val) {
        value_ = forward<U>(val);
        valid = true;
    }

    T& operator*() {
        return value_;
    }
    const T& operator*() const {
        return value_;
    }

    T* operator->() {
        return &value_;
    }
    const T* operator->() const {
        return &value_;
    }

    operator bool() const {
        return valid;
    }

  private:
    T value_;
    bool valid = false;
};

}  // namespace pstd

#endif  // PINE_STD_OPTIONAL_H