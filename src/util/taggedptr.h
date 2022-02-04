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

    template <int index>
    static constexpr auto ToPtr(void* ptr) {
        return (T*)ptr;
    }

    template <int index>
    static constexpr auto ToPtrConst(const void* ptr) {
        return (const T*)ptr;
    }

    template <int index, typename... Args>
    static constexpr auto ToConcrete(Args&&... args) {
        return T(std::forward<Args...>(args...));
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

    template <int index>
    static constexpr auto ToPtr(void* ptr) {
        if constexpr (index == 0)
            return (T*)ptr;
        else
            return TypePack<Ts...>::template ToPtr<index - 1>(ptr);
    }
    template <int index>
    static constexpr auto ToPtrConst(const void* ptr) {
        if constexpr (index == 0)
            return (const T*)ptr;
        else
            return TypePack<Ts...>::template ToPtrConst<index - 1>(ptr);
    }

    template <int index, typename... Args>
    static constexpr auto ToConcrete(Args&&... args) {
        if constexpr (index == 0)
            return T(std::forward<Args...>(args...));
        else
            return TypePack<Ts...>::template ToConcrete<index - 1>(std::forward<Args...>(args...));
    }
};

template <typename... Ts>
struct TaggedPointer {
    using type = TypePack<Ts...>;

    TaggedPointer() = default;

    template <typename T>
    TaggedPointer(T* ptr) {
        CHECK(bits == 0);
        CHECK((uint64_t)ptr == ((uint64_t)ptr & ptrMask));
        bits = (uint64_t)ptr;
        bits |= uint64_t(type::template Index<T>()) << tagShift;
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
        if (Tag() != type::template Index<U>())
            return nullptr;
        return (U*)Ptr();
    }
    template <typename U>
    const U* Cast() const{
        if (Tag() != type::template Index<U>())
            return nullptr;
        return (U*)Ptr();
    }

    template <typename F>
    decltype(auto) Dispatch(F func) {
        CHECK(Ptr() != nullptr);
        switch (Tag()) {
        case 0: return func(type::template ToPtr<0>(Ptr()));
        case 1: return func(type::template ToPtr<1>(Ptr()));
        case 2: return func(type::template ToPtr<2>(Ptr()));
        case 3: return func(type::template ToPtr<3>(Ptr()));
        case 4: return func(type::template ToPtr<4>(Ptr()));
        case 5: return func(type::template ToPtr<5>(Ptr()));
        case 6: return func(type::template ToPtr<6>(Ptr()));
        default: return func(type::template ToPtr<7>(Ptr()));
        }
    }
    template <typename F>
    decltype(auto) Dispatch(F func) const {
        CHECK(Ptr() != nullptr);
        switch (Tag()) {
        case 0: return func(type::template ToPtrConst<0>(Ptr()));
        case 1: return func(type::template ToPtrConst<1>(Ptr()));
        case 2: return func(type::template ToPtrConst<2>(Ptr()));
        case 3: return func(type::template ToPtrConst<3>(Ptr()));
        case 4: return func(type::template ToPtrConst<4>(Ptr()));
        case 5: return func(type::template ToPtrConst<5>(Ptr()));
        case 6: return func(type::template ToPtrConst<6>(Ptr()));
        default: return func(type::template ToPtrConst<7>(Ptr()));
        }
    }

    template <typename... Args, typename F>
    static decltype(auto) DispatchConcrete(int tag, F func, Args&&... args) {
        switch (tag) {
        case 0: return func(type::template ToConcrete<0>(std::forward<Args...>(args...)));
        case 1: return func(type::template ToConcrete<1>(std::forward<Args...>(args...)));
        case 2: return func(type::template ToConcrete<2>(std::forward<Args...>(args...)));
        case 3: return func(type::template ToConcrete<3>(std::forward<Args...>(args...)));
        case 4: return func(type::template ToConcrete<4>(std::forward<Args...>(args...)));
        case 5: return func(type::template ToConcrete<5>(std::forward<Args...>(args...)));
        case 6: return func(type::template ToConcrete<6>(std::forward<Args...>(args...)));
        default: return func(type::template ToConcrete<7>(std::forward<Args...>(args...)));
        }
    }

    void InvokeDestructor() {
        if (bits) {
            Dispatch([](auto ptr) {
                using T = std::decay_t<decltype(*ptr)>;
                (ptr->~T)();
            });
        }
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