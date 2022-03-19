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

template <typename BufferType, typename Strategy, bool RenewPtr>
class ArchiverBase {
  public:
    ArchiverBase() = default;
    ArchiverBase(BufferType data) : data(std::move(data)){};

    template <typename T, typename... Ts>
    const BufferType& Archive(T& first, Ts&... rest) {
        ArchiveImpl(first);
        if constexpr (sizeof...(rest) != 0)
            Archive(rest...);
        return data;
    }

    template <typename Ty>
    void ArchiveImpl(Ty& object) {
        using T = std::decay_t<Ty>;

        if constexpr (std::is_trivial<T>::value) {
            strategy.Add(object);

        } else if constexpr (IsMap<T>::value) {
            strategy.Add(object);
            if constexpr (RenewPtr)
                new (&object) T(object.size());

            for (const auto& item : object) {
                ArchiveImpl(item.first);
                ArchiveImpl(item.second);
            }

        } else if constexpr (IsVector<T>::value) {
            strategy.Add(object);
            if constexpr (RenewPtr)
                new (&object) T(object.size());

            for (size_t i = 0; i < object.size(); i++)
                ArchiveImpl(object[i]);

        } else if constexpr (IsPointer<T>::value) {
            using Tv = std::decay_t<decltype(*object)>;
            if constexpr (RenewPtr)
                object = T(new Tv);

            ArchiveImpl(*object);

        } else {
            ForEachField(object, [&](auto&& field) { ArchiveImpl(field); });
        }
    }

  private:
    BufferType data;
    Strategy strategy{data};
};

using ArchiveBufferType = std::vector<char>;
using Serializer = ArchiverBase<ArchiveBufferType, ArchiveWriter<ArchiveBufferType>, false>;
using Deserializer = ArchiverBase<ArchiveBufferType, ArchiveReader<ArchiveBufferType>, true>;

}  // namespace pine

#endif  // PINE_UTIL_ARCHIVE_H