#ifndef PINE_UTIL_PARAMETER_H
#define PINE_UTIL_PARAMETER_H

#include <core/vecmath.h>
#include <util/log.h>

#include <pstd/map.h>
#include <pstd/string.h>
#include <pstd/vector.h>
#include <pstd/optional.h>

namespace pine {

struct Parameters {
    void Summarize(int indent = 0) const;

    template <typename T>
    Parameters& Set(pstd::string name, T val) {
        values[name] = pstd::to_string(val);
        return *this;
    }

    bool HasValue(const pstd::string& name) const;
    bool HasSubset(const pstd::string& name) const;

    pstd::optional<bool> TryGetBool(const pstd::string& name) const;
    pstd::optional<int> TryGetInt(const pstd::string& name) const;
    pstd::optional<float> TryGetFloat(const pstd::string& name) const;
    pstd::optional<vec2i> TryGetVec2i(const pstd::string& name) const;
    pstd::optional<vec3i> TryGetVec3i(const pstd::string& name) const;
    pstd::optional<vec4i> TryGetVec4i(const pstd::string& name) const;
    pstd::optional<vec2> TryGetVec2(const pstd::string& name) const;
    pstd::optional<vec3> TryGetVec3(const pstd::string& name) const;
    pstd::optional<vec4> TryGetVec4(const pstd::string& name) const;
    pstd::optional<pstd::string> TryGetString(const pstd::string& name) const;

#define DefineGetX(R, Type)                                                 \
    R Get##Type(const pstd::string& name) const {                           \
        if (auto value = TryGet##Type(name))                                \
            return *value;                                                  \
        else                                                                \
            LOG_FATAL("[Parameters][Get" #Type "]cannot find \"&\"", name); \
        return {};                                                          \
    }                                                                       \
    R Get##Type(const pstd::string& name, const R& fallback) const {        \
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
    DefineGetX(pstd::string, String)
    // clang-format on
#undef DefineGetX

                const pstd::vector<Parameters>& GetAll(pstd::string name) const;
    Parameters& AddSubset(pstd::string name);

    Parameters& operator[](pstd::string name);
    const Parameters& operator[](pstd::string name) const;

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

    pstd::map<pstd::string, pstd::string> values;
    mutable pstd::map<pstd::string, pstd::vector<Parameters>> subset;
};  // namespace pine

}  // namespace pine

#endif  // PINE_UTIL_PARAMETER_H