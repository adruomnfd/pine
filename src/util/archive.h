#ifndef PINE_UTIL_ARCHIVE_H
#define PINE_UTIL_ARCHIVE_H

#include <core/defines.h>
#include <util/reflect.h>

#include <vector>
#include <memory>

#include <cstring>

namespace pine {

template <typename BufferType>
struct ArchiveWriter {
    ArchiveWriter(BufferType& data) : data(data){};

    template <typename T>
    void Add(const T& value) {
        size_t size = sizeof(value);
        data.resize(data.size() + size);
        char* ptr = data.data() + data.size() - size;
        memcpy(ptr, &value, size);
    }

    BufferType& data;
};

template <typename BufferType>
struct ArchiveReader {
    ArchiveReader(BufferType& data) : data(data){};

    template <typename T>
    void Add(T& value) {
        size_t size = sizeof(value);
        memcpy(&value, data.data() + offset, size);
        offset += size;
    }

    BufferType& data;
    size_t offset = 0;
};

template <typename BufferType, typename Strategy, bool IsUnarchive>
class ArchiverBase {
  public:
    ArchiverBase() = default;
    ArchiverBase(BufferType data) : data(std::move(data)){};

    template <typename T>
    T Unarchive() {
        T object;
        ArchiveImpl(object);
        return object;
    }

    template <typename... Ts, typename = std::enable_if_t<(sizeof...(Ts) > 1)>>
    auto Unarchive() {
        std::tuple<Ts...> object;
        ArchiveImpl(object);
        return object;
    }

    template <typename T, typename... Ts>
    const BufferType& Archive(T&& first, Ts&&... rest) {
        ArchiveImpl(std::forward<T>(first));
        if constexpr (sizeof...(rest) != 0)
            Archive(std::forward<Ts>(rest)...);
        return data;
    }

    template <typename Ty>
    void ArchiveImpl(Ty&& object) {
        using T = std::decay_t<Ty>;

        if constexpr (std::is_trivial<T>::value) {
            strategy.Add(std::forward<Ty>(object));

        } else if constexpr (IsMap<T>::value) {
            strategy.Add(std::forward<Ty>(object));
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
            strategy.Add(std::forward<Ty>(object));
            if constexpr (IsUnarchive) {
                size_t size = object.size();
                new (&object) T;
                object.resize(size);
            }

            for (size_t i = 0; i < object.size(); i++)
                ArchiveImpl(std::forward<Ty>(object)[i]);

        } else if constexpr (IsPointer<T>::value) {
            using Tv = std::decay_t<decltype(*object)>;
            if constexpr (IsUnarchive)
                object = T(new Tv);

            ArchiveImpl(*std::forward<Ty>(object));

        } else {
            ForEachField(std::forward<Ty>(object),
                         [&](auto&& field) { ArchiveImpl(std::forward<decltype(field)>(field)); });
        }
    }

  private:
    BufferType data;
    Strategy strategy{data};
};

using ArchiveBufferType = std::vector<char>;
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
    return Deserializer(std::move(data)).Unarchive<Ts...>();
}

}  // namespace pine

#endif  // PINE_UTIL_ARCHIVE_H