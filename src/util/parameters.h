#ifndef PINE_UTIL_PARAMETER_H
#define PINE_UTIL_PARAMETER_H

#include <core/vecmath.h>
#include <util/log.h>

#include <map>
#include <string>
#include <vector>

namespace pine {

struct Parameters {
    void Summarize(int indent = 0) const;

    template <typename T>
    Parameters& Set(std::string name, T val) {
        values[name] = ToString(val);
        return *this;
    }

    bool HasValue(const std::string& name) const;
    bool HasSubset(const std::string& name) const;

    std::optional<bool> TryGetBool(const std::string& name) const;
    std::optional<int> TryGetInt(const std::string& name) const;
    std::optional<float> TryGetFloat(const std::string& name) const;
    std::optional<vec2i> TryGetVec2i(const std::string& name) const;
    std::optional<vec3i> TryGetVec3i(const std::string& name) const;
    std::optional<vec4i> TryGetVec4i(const std::string& name) const;
    std::optional<vec2> TryGetVec2(const std::string& name) const;
    std::optional<vec3> TryGetVec3(const std::string& name) const;
    std::optional<vec4> TryGetVec4(const std::string& name) const;
    std::optional<std::string> TryGetString(const std::string& name) const;

#define DefineGetX(R, Type)                                                 \
    R Get##Type(const std::string& name) const {                            \
        if (auto value = TryGet##Type(name))                                \
            return *value;                                                  \
        else                                                                \
            LOG_FATAL("[Parameters][Get" #Type "]cannot find \"&\"", name); \
        return {};                                                          \
    }                                                                       \
    R Get##Type(const std::string& name, const R& fallback) const {         \
        if (auto value = TryGet##Type(name))                                \
            return *value;                                                  \
        else                                                                \
            return fallback;                                                \
    }
    // clang-format off
    DefineGetX(bool, Bool)
    DefineGetX(int, Int)
    DefineGetX(float, Float)
    DefineGetX(vec2i, Vec2i)
    DefineGetX(vec3i, Vec3i)
    DefineGetX(vec4i, Vec4i)
    DefineGetX(vec2, Vec2)
    DefineGetX(vec3, Vec3)
    DefineGetX(vec4, Vec4)
    DefineGetX(std::string, String)
    // clang-format on
#undef DefineGetX

                const std::vector<Parameters>& GetAll(std::string name) const;
    Parameters& AddSubset(std::string name);

    Parameters& operator[](std::string name);
    const Parameters& operator[](std::string name) const;

    auto begin() {
        return subset.begin();
    }
    auto begin() const {
        return subset.begin();
    }
    auto end() {
        return subset.end();
    }
    auto end() const {
        return subset.end();
    }

    std::map<std::string, std::string> values;
    mutable std::map<std::string, std::vector<Parameters>> subset;
};  // namespace pine

}  // namespace pine

#endif  // PINE_UTIL_PARAMETER_H