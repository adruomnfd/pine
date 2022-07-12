#ifndef PINE_UTIL_ARCHIVE_H
#define PINE_UTIL_ARCHIVE_H

#include <core/defines.h>
#include <util/reflect.h>

#include <pstd/vector.h>
#include <pstd/memory.h>

namespace pine {

template <typename BufferType>
struct ArchiveWriter {
    ArchiveWriter(BufferType& data) : data(data){};

    template <typename T>
    void Add(const T& value) {
        size_t size = sizeof(value);
        data.resize(data.size() + size);
        char* ptr = data.data() + data.size() - size;
        pstd::memcpy(ptr, &value, size);
    }

    BufferType& data;
};

template <typename BufferType>
struct ArchiveReader {
    ArchiveReader(BufferType& data) : data(data){};

    template <typename T>
    void Add(T& value) {
        size_t size = sizeof(value);
        pstd::memcpy(&value, data.data() + offset, size);
        offset += size;
    }

    BufferType& data;
    size_t offset = 0;
};

template <typename BufferType, typename Strategy, bool IsUnarchive>
class ArchiverBase {
  public:
    ArchiverBase() = default;
    ArchiverBase(BufferType data) : data(pstd::move(data)){};

    template <typename T>
    T Unarchive() {
        T object;
        ArchiveImpl(object);
        return object;
    }

    template <typename... Ts, typename = pstd::enable_if_t<(sizeof...(Ts) > 1)>>
    auto Unarchive() {
        pstd::tuple<Ts...> object;
        ArchiveImpl(object);
        return object;
    }

    template <typename T, typename... Ts>
    const BufferType& Archive(T&& first, Ts&&... rest) {
        ArchiveImpl(pstd::forward<T>(first));
        if constexpr (sizeof...(rest) != 0)
            Archive(pstd::forward<Ts>(rest)...);
        return data;
    }

    template <typename Ty>
    void ArchiveImpl(Ty&& object) {
        using T = pstd::decay_t<Ty>;

        if constexpr (pstd::is_arithmetic_v<T>) {
            strategy.Add(pstd::forward<Ty>(object));

        } else if constexpr (IsMap<T>::value) {
            strategy.Add(pstd::forward<Ty>(object));
            if constexpr (IsUnarchive) {
                size_t size = object.size();
                new (&object) T;
                for (size_t i = 0; i < size; i++) {
                    typename T::key_type key;
                    typename T::mapped_type value;
                    ArchiveImpl(key);
                    ArchiveImpl(value);
                    object[key] = value;
                }
            } else {
                for (const auto& item : object) {
                    ArchiveImpl(item.first);
                    ArchiveImpl(item.second);
                }
            }

        } else if constexpr (IsVector<T>::value) {
            strategy.Add(pstd::forward<Ty>(object));
            if constexpr (IsUnarchive) {
                size_t size = object.size();
                new (&object) T;
                object.resize(size);
            }

            for (size_t i = 0; i < object.size(); i++)
                ArchiveImpl(pstd::forward<Ty>(object)[i]);

        } else if constexpr (pstd::is_pointerish_v<T>) {
            using Tv = pstd::decay_t<decltype(*object)>;
            if constexpr (IsUnarchive)
                object = T(new Tv);

            ArchiveImpl(*pstd::forward<Ty>(object));

        } else {
            pstd::foreach_field(pstd::forward<Ty>(object), [&](auto&& field) {
                ArchiveImpl(pstd::forward<decltype(field)>(field));
            });
        }
    }

  private:
    BufferType data;
    Strategy strategy{data};
};

using ArchiveBufferType = pstd::vector<char>;
using Serializer = ArchiverBase<ArchiveBufferType, ArchiveWriter<ArchiveBufferType>, false>;
using Deserializer = ArchiverBase<ArchiveBufferType, ArchiveReader<ArchiveBufferType>, true>;

template <typename... Ts>
auto Archive(const Ts&... input) {
    return Serializer().Archive(input...);
}

template <typename... Ts>
auto Unarchive(const ArchiveBufferType& data) {
    return Deserializer(data).Unarchive<Ts...>();
}

template <typename... Ts>
auto Unarchive(ArchiveBufferType&& data) {
    return Deserializer(pstd::move(data)).Unarchive<Ts...>();
}

}  // namespace pine

#endif  // PINE_UTIL_ARCHIVE_H