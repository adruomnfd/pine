#ifndef PINE_UTIL_PARAMETER_H
#define PINE_UTIL_PARAMETER_H

#include <core/vecmath.h>

#include <map>
#include <string>
#include <vector>

namespace pine {

struct Parameters {
    void Override(const Parameters& it);
    void Summarize(int indent = 0) const;

    template <typename T>
    void Set(std::string name, T val) {
        values[name] = ToString(val);
    }
    template <typename T>
    void SetSubset(std::string subsetName, std::string name, T val) {
        subset[subsetName].Set(name, val);
    }

    bool Has(const std::string& name) const;
    bool GetBool(const std::string& name, bool fallback = {}) const;
    int GetInt(const std::string& name, int fallback = {}) const;
    float GetFloat(const std::string& name, float fallback = {}) const;
    vec2i GetVec2i(const std::string& name, vec2i fallback = {}) const;
    vec3i GetVec3i(const std::string& name, vec3i fallback = {}) const;
    vec4i GetVec4i(const std::string& name, vec4i fallback = {}) const;
    vec2 GetVec2(const std::string& name, vec2 fallback = {}) const;
    vec3 GetVec3(const std::string& name, vec3 fallback = {}) const;
    vec4 GetVec4(const std::string& name, vec4 fallback = {}) const;
    std::string GetString(const std::string& name, const std::string& fallback = {}) const;
    const Parameters& operator[](std::string name) const {
        return subset[name];
    }
    Parameters& operator[](std::string name) {
        return subset[name];
    }
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
    mutable std::map<std::string, Parameters> subset;
};

}  // namespace pine

#endif  // PINE_UTIL_PARAMETER_H