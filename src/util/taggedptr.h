#ifndef PINE_UTIL_TAGGEDPTR_H
#define PINE_UTIL_TAGGEDPTR_H

#include <util/log.h>

#include <utility>

namespace pine {

template <typename... Ts>
struct TypePack;

template <typename T>
struct TypePack<T> {
    template <typename Ty>
    static constexpr int Index() {
        static_assert(std::is_same<T, Ty>(), "Type \"Ty\" is not a member of this TypePack");
        return 0;
    }
};

template <typename T, typename... Ts>
struct TypePack<T, Ts...> : TypePack<Ts...> {
    template <typename Ty>
    static constexpr int Index() {
        if constexpr (std::is_same<T, Ty>())
            return 0;
        else
            return 1 + TypePack<Ts...>::template Index<Ty>();
    }
};

template <typename T0, typename F>
decltype(auto) Dispatch(F&& func, int tag, void* ptr) {
    switch (tag) {
    case 0: return func((T0*)ptr);
    }
    UNREACHABLE;
}

template <typename T0, typename T1, typename F>
decltype(auto) Dispatch(F&& func, int tag, void* ptr) {
    switch (tag) {
    case 0: return func((T0*)ptr);
    case 1: return func((T1*)ptr);
    }
    UNREACHABLE;
}

template <typename T0, typename T1, typename T2, typename F>
decltype(auto) Dispatch(F&& func, int tag, void* ptr) {
    switch (tag) {
    case 0: return func((T0*)ptr);
    case 1: return func((T1*)ptr);
    case 2: return func((T2*)ptr);
    }
    UNREACHABLE;
}

template <typename T0, typename T1, typename T2, typename T3, typename F>
decltype(auto) Dispatch(F&& func, int tag, void* ptr) {
    switch (tag) {
    case 0: return func((T0*)ptr);
    case 1: return func((T1*)ptr);
    case 2: return func((T2*)ptr);
    case 3: return func((T3*)ptr);
    }
    UNREACHABLE;
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename F>
decltype(auto) Dispatch(F&& func, int tag, void* ptr) {
    switch (tag) {
    case 0: return func((T0*)ptr);
    case 1: return func((T1*)ptr);
    case 2: return func((T2*)ptr);
    case 3: return func((T3*)ptr);
    case 4: return func((T4*)ptr);
    }
    UNREACHABLE;
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename F>
decltype(auto) Dispatch(F&& func, int tag, void* ptr) {
    switch (tag) {
    case 0: return func((T0*)ptr);
    case 1: return func((T1*)ptr);
    case 2: return func((T2*)ptr);
    case 3: return func((T3*)ptr);
    case 4: return func((T4*)ptr);
    case 5: return func((T5*)ptr);
    }
    UNREACHABLE;
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
          typename F>
decltype(auto) Dispatch(F&& func, int tag, void* ptr) {
    switch (tag) {
    case 0: return func((T0*)ptr);
    case 1: return func((T1*)ptr);
    case 2: return func((T2*)ptr);
    case 3: return func((T3*)ptr);
    case 4: return func((T4*)ptr);
    case 5: return func((T5*)ptr);
    case 6: return func((T6*)ptr);
    }
    UNREACHABLE;
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
          typename T7, typename F>
decltype(auto) Dispatch(F&& func, int tag, void* ptr) {
    switch (tag) {
    case 0: return func((T0*)ptr);
    case 1: return func((T1*)ptr);
    case 2: return func((T2*)ptr);
    case 3: return func((T3*)ptr);
    case 4: return func((T4*)ptr);
    case 5: return func((T5*)ptr);
    case 6: return func((T6*)ptr);
    case 7: return func((T7*)ptr);
    }
    UNREACHABLE;
}

template <typename T0, typename F>
decltype(auto) DispatchConst(F&& func, int tag, const void* ptr) {
    switch (tag) {
    case 0: return func((const T0*)ptr);
    }
    UNREACHABLE;
}

template <typename T0, typename T1, typename F>
decltype(auto) DispatchConst(F&& func, int tag, const void* ptr) {
    switch (tag) {
    case 0: return func((const T0*)ptr);
    case 1: return func((const T1*)ptr);
    }
    UNREACHABLE;
}

template <typename T0, typename T1, typename T2, typename F>
decltype(auto) DispatchConst(F&& func, int tag, const void* ptr) {
    switch (tag) {
    case 0: return func((const T0*)ptr);
    case 1: return func((const T1*)ptr);
    case 2: return func((const T2*)ptr);
    }
    UNREACHABLE;
}

template <typename T0, typename T1, typename T2, typename T3, typename F>
decltype(auto) DispatchConst(F&& func, int tag, const void* ptr) {
    switch (tag) {
    case 0: return func((const T0*)ptr);
    case 1: return func((const T1*)ptr);
    case 2: return func((const T2*)ptr);
    case 3: return func((const T3*)ptr);
    }
    UNREACHABLE;
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename F>
decltype(auto) DispatchConst(F&& func, int tag, const void* ptr) {
    switch (tag) {
    case 0: return func((const T0*)ptr);
    case 1: return func((const T1*)ptr);
    case 2: return func((const T2*)ptr);
    case 3: return func((const T3*)ptr);
    case 4: return func((const T4*)ptr);
    }
    UNREACHABLE;
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename F>
decltype(auto) DispatchConst(F&& func, int tag, const void* ptr) {
    switch (tag) {
    case 0: return func((const T0*)ptr);
    case 1: return func((const T1*)ptr);
    case 2: return func((const T2*)ptr);
    case 3: return func((const T3*)ptr);
    case 4: return func((const T4*)ptr);
    case 5: return func((const T5*)ptr);
    }
    UNREACHABLE;
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
          typename F>
decltype(auto) DispatchConst(F&& func, int tag, const void* ptr) {
    switch (tag) {
    case 0: return func((const T0*)ptr);
    case 1: return func((const T1*)ptr);
    case 2: return func((const T2*)ptr);
    case 3: return func((const T3*)ptr);
    case 4: return func((const T4*)ptr);
    case 5: return func((const T5*)ptr);
    case 6: return func((const T6*)ptr);
    }
    UNREACHABLE;
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
          typename T7, typename F>
decltype(auto) DispatchConst(F&& func, int tag, const void* ptr) {
    switch (tag) {
    case 0: return func((const T0*)ptr);
    case 1: return func((const T1*)ptr);
    case 2: return func((const T2*)ptr);
    case 3: return func((const T3*)ptr);
    case 4: return func((const T4*)ptr);
    case 5: return func((const T5*)ptr);
    case 6: return func((const T6*)ptr);
    case 7: return func((const T7*)ptr);
    }
    UNREACHABLE;
}

template <typename... Ts>
struct TaggedPointer {
    using types = TypePack<Ts...>;

    static void Destory(TaggedPointer ptr) {
        ptr.Delete();
    }

    TaggedPointer() = default;
    template <typename T>
    TaggedPointer(T* ptr) {
        CHECK(bits == 0);
        CHECK((uint64_t)ptr == ((uint64_t)ptr & ptrMask));
        bits = (uint64_t)ptr;
        bits |= uint64_t(types::template Index<T>()) << tagShift;
    }

    TaggedPointer(const TaggedPointer&) = default;
    TaggedPointer& operator=(const TaggedPointer&) = default;
    TaggedPointer(TaggedPointer&& rhs) {
        bits = rhs.bits;
        rhs.bits = 0;
    }
    TaggedPointer& operator=(TaggedPointer&& rhs) {
        bits = rhs.bits;
        rhs.bits = 0;
        return *this;
    }
    friend bool operator==(TaggedPointer lhs, TaggedPointer rhs) {
        return lhs.Ptr() == rhs.Ptr();
    }
    friend bool operator!=(TaggedPointer lhs, TaggedPointer rhs) {
        return lhs.Ptr() != rhs.Ptr();
    }

    int Tag() const {
        return int((bits & tagMask) >> tagShift);
    }

    void* Ptr() {
        return (void*)(bits & ptrMask);
    }
    const void* Ptr() const {
        return (const void*)(bits & ptrMask);
    }
    operator void*() {
        return Ptr();
    }
    operator const void*() const {
        return Ptr();
    }
    template <typename U>
    U* Cast() {
        if (Tag() != types::template Index<U>())
            return nullptr;
        return (U*)Ptr();
    }
    template <typename U>
    const U* Cast() const {
        if (Tag() != types::template Index<U>())
            return nullptr;
        return (U*)Ptr();
    }

    template <typename F>
    decltype(auto) Dispatch(F&& f) {
        CHECK(Ptr() != nullptr);
        return pine::Dispatch<Ts...>(f, Tag(), Ptr());
    }
    template <typename F>
    decltype(auto) Dispatch(F&& f) const {
        CHECK(Ptr() != nullptr);
        return pine::DispatchConst<Ts...>(f, Tag(), Ptr());
    }

    void InvokeDestructor() {
        if (bits) {
            Dispatch([](auto ptr) {
                using T = std::decay_t<decltype(*ptr)>;
                (ptr->~T)();
            });
        }
    }

    TaggedPointer Clone() const {
        Dispatch([](auto ptr) {
            using T = std::decay_t<decltype(*ptr)>;
            return new T(*ptr);
        });
    }

    void Delete() {
        if (bits) {
            Dispatch([](auto ptr) { delete ptr; });
            bits = 0;
        }
    }

    template <typename... Args>
    void Reset(Args&&... args) {
        Dispatch([&](auto ptr) { *ptr = {std::forward<Args>(args)...}; });
    }

    template <typename ArchiveT>
    void Archive(ArchiveT& archive) {
        archive(bits);
        Dispatch([&](auto ptr) {
            archive(ptr);
            *this = ptr;
        });
    }
    template <typename ArchiveT>
    void Archive(ArchiveT& archive) const {
        archive(bits);
        Dispatch([&](auto ptr) { archive(ptr); });
    }
    static constexpr bool Archivable = true;

  protected:
    static_assert(sizeof(uintptr_t) == 8, "Expect sizeof(uint64_t) to be 64 bits");
    static constexpr int tagShift = 57;
    static constexpr int tagBits = 64 - tagShift;
    static constexpr uint64_t tagMask = ((1ull << tagBits) - 1) << tagShift;
    static constexpr uint64_t ptrMask = ~tagMask;
    uint64_t bits = 0;
};

}  // namespace pine

#endif  // PINE_UTIL_TAGGEDPTR_H