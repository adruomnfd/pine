#ifndef PINE_UTIL_PARAMETER_H
#define PINE_UTIL_PARAMETER_H

#include <core/vecmath.h>

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
};

}  // namespace pine

#endif  // PINE_UTIL_PARAMETER_H