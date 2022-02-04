#ifndef PINE_UTIL_SERIALIZER_H
#define PINE_UTIL_SERIALIZER_H

#include <core/defines.h>

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>

namespace pine {

template <typename T>
struct IsVector {
    template <typename U>
    static std::true_type Check(decltype(&U::size));
    template <typename U>
    static std::false_type Check(...);

    static constexpr bool value = std::is_same<decltype(Check<T>(0)), std::true_type>::value;
};

template <typename T>
struct IsMap {
    template <typename U>
    static std::true_type Check(typename U::key_type);
    template <typename U>
    static std::false_type Check(...);

    static constexpr bool value = std::is_same<decltype(Check<T>(0)), std::true_type>::value;
};

template <typename T>
struct IsArchivable {
    template <typename U>
    static std::true_type Check(decltype(&U::Archivable));
    template <typename U>
    static std::false_type Check(...);

    static constexpr bool value = std::is_same<decltype(Check<T>(0)), std::true_type>::value;
};

template <typename T>
struct IsSmartPointer : std::false_type {};
template <typename T>
struct IsSmartPointer<std::shared_ptr<T>> : std::true_type {};
template <typename T>
struct IsSmartPointer<std::unique_ptr<T>> : std::true_type {};

class Serializer {
  public:
    template <typename... Ts>
    void operator()(const Ts&... objects) {
        Archive(objects...);
    }

    template <typename T, typename... Ts>
    void Archive(const T& first, const Ts&... rest) {
        Archive(first);
        Archive(rest...);
    }

    template <typename T>
    void Archive(const T& object) {
        static_assert(std::is_fundamental<T>::value || std::is_enum<T>::value ||
                          IsArchivable<T>::value || IsVector<T>::value || IsMap<T>::value ||
                          IsSmartPointer<T>::value || std::is_pointer<T>::value ||
                          std::is_empty<T>::value,
                      "Type is not archivable");

        if constexpr (std::is_fundamental<T>::value || std::is_enum<T>::value) {
            data.resize(data.size() + sizeof(T));
            memcpy(data.data() + data.size() - sizeof(T), &object, sizeof(T));
        } else if constexpr (IsArchivable<T>::value) {
            object.Archive(*this);
        } else if constexpr (IsMap<T>::value) {
            size_t size = object.size();
            data.resize(data.size() + sizeof(size));
            memcpy(data.data() + data.size() - sizeof(size), &size, sizeof(size));
            for (const auto& item : object) {
                Archive(item.first);
                Archive(item.second);
            }
        } else if constexpr (IsVector<T>::value) {
            size_t size = object.size();
            data.resize(data.size() + sizeof(size));
            memcpy(data.data() + data.size() - sizeof(size), &size, sizeof(size));

            if constexpr (IsArchivable<std::decay_t<decltype(object[0])>>::value) {
                for (size_t i = 0; i < object.size(); i++)
                    object[i].Archive(*this);
            } else {
                data.resize(data.size() + object.size() * sizeof(object[0]));
                memcpy(data.data() + data.size() - object.size() * sizeof(object[0]), &object[0],
                       object.size() * sizeof(object[0]));
            }
        } else if constexpr (std::is_pointer<T>::value) {
            if (pointers.find((void*)object) == pointers.end()) {
                pointers.insert((void*)object);
                Archive(*object);
            }
        } else if constexpr (IsSmartPointer<T>::value) {
            if (pointers.find((void*)object.get()) == pointers.end()) {
                pointers.insert((void*)object.get());
                Archive(*object);
            }
        }
    }

    std::unordered_set<void*> pointers;
    std::vector<char> data;
};

class Deserializer {
  public:
    Deserializer(std::vector<char> data) : data(std::move(data)) {
    }

    template <typename... Ts>
    void operator()(Ts&... objects) {
        Archive(objects...);
    }

    template <typename T, typename... Ts>
    void Archive(T& first, Ts&... rest) {
        Archive(first);
        Archive(rest...);
    }

    template <typename T>
    void Archive(T& object) {
        static_assert(std::is_fundamental<T>::value || std::is_enum<T>::value ||
                          IsArchivable<T>::value || IsVector<T>::value || IsMap<T>::value ||
                          IsSmartPointer<T>::value || std::is_pointer<T>::value ||
                          std::is_empty<T>::value,
                      "Type is not archivable");

        if constexpr (std::is_fundamental<T>::value || std::is_enum<T>::value) {
            memcpy(&object, data.data() + offset, sizeof(T));
            offset += sizeof(T);
        } else if constexpr (IsArchivable<T>::value) {
            object.Archive(*this);
        } else if constexpr (IsMap<T>::value) {
            size_t size = 0;
            memcpy(&size, data.data() + offset, sizeof(size));
            offset += sizeof(size);

            for (size_t i = 0; i < size; i++) {
                std::decay_t<decltype(object.begin()->first)> key;
                Archive(key);
                Archive(object[key]);
            }
        } else if constexpr (IsVector<T>::value) {
            size_t size = 0;
            memcpy(&size, data.data() + offset, sizeof(size));
            object.resize(size);
            offset += sizeof(size);

            if constexpr (IsArchivable<std::decay_t<decltype(object[0])>>::value) {
                for (size_t i = 0; i < object.size(); i++)
                    object[i].Archive(*this);
            } else {
                if (size) {
                    memcpy((void*)&object[0], data.data() + offset,
                           object.size() * sizeof(object[0]));
                    offset += object.size() * sizeof(object[0]);
                }
            }
        } else if constexpr (std::is_pointer<T>::value) {
            auto iter = pointersMap.find(object);
            if (iter == pointersMap.end()) {
                void* raw = object;
                object = new (std::decay_t<decltype(*object)>)(std::decay_t<decltype(*object)>());
                pointersMap[raw] = object;
                Archive(*object);
            } else {
                object = (decltype(object))iter->second;
            }
        } else if constexpr (IsSmartPointer<T>::value) {
            auto iter = pointersMap.find(object.get());
            if (iter == pointersMap.end()) {
                void* raw = object.get();
                object.reset(
                    new (std::decay_t<decltype(*object)>)(std::decay_t<decltype(*object)>()));
                pointersMap[raw] = object.get();
                Archive(*object);
            } else {
                object.reset((decltype(object))iter->second);
            }
        }
    }

  private:
    std::vector<char> data;
    std::unordered_map<void*, void*> pointersMap;
    size_t offset = 0;
};

}  // namespace pine

#endif  // PINE_UTIL_SERIALIZER_H