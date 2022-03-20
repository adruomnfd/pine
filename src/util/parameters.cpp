#include <util/parameters.h>
#include <util/log.h>

#include <sstream>

namespace pine {

void Parameters::Summarize(int indent) const {
    for (const auto& v : values)
        LOG_VERBOSE("& &: &", Format(indent), " ", v.first.c_str(), v.second.c_str());
    for (const auto& s : subset) {
        for (auto& ss : s.second) {
            LOG("& &", Format(indent), " ", s.first);
            ss.Summarize(indent + 2);
        }
    }
}

const std::vector<Parameters>& Parameters::GetAll(std::string name) const {
    return subset[name];
}
Parameters& Parameters::AddSubset(std::string name) {
    auto& sub = subset[name];
    sub.resize(sub.size() + 1);
    return sub.back();
}

Parameters& Parameters::operator[](std::string name) {
    static Parameters fallback;
    auto& ss = subset[name];

    if (HasValue(name) && ss.size() == 0)
        AddSubset(name).Set("@", GetString(name));

    if (ss.size() == 0) {
        LOG_WARNING("[Parameters][GetSubset]No subset named \"&\"", name);
        return fallback;
    }
    if (ss.size() != 1) {
        LOG_WARNING("[Parameters][GetSubset]Find & subset with name \"&\", returns the last one",
                    subset[name].size(), name);
    }
    return subset[name].back();
}
const Parameters& Parameters::operator[](std::string name) const {
    return (*const_cast<Parameters*>(this))[name];
}

bool Parameters::HasValue(const std::string& name) const {
    return values.find(name) != values.end();
}

bool Parameters::HasSubset(const std::string& name) const {
    return subset.find(name) != subset.end();
}

bool Parameters::GetBool(const std::string& name, bool fallback) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return fallback;
    std::string str = iter->second;
    for (size_t i = 0; i < str.size(); i++)
        if ('A' <= str[i] && str[i] <= 'Z')
            str[i] += 'a' - 'A';

    if (str == "true")
        return true;
    else if (str == "false")
        return false;
    else
        return (bool)std::stoi(str);
}
int Parameters::GetInt(const std::string& name, int fallback) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return fallback;
    std::string str = iter->second;
    if (str == "") {
        LOG_WARNING("[Parameters][GetInt]& do not have a value", name.c_str());
        return fallback;
    }

    int val = 0;
    try {
        val = std::stoi(str);
    } catch (const std::exception& e) {
        LOG_FATAL("[Parameters][GetInt]cannot convert \"&\" to an int", str);
    }
    return val;
}
float Parameters::GetFloat(const std::string& name, float fallback) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return fallback;
    std::string str = iter->second;
    if (str == "") {
        LOG_WARNING("[Parameters][GetFloat]& do not have a value", name.c_str());
        return fallback;
    }

    float val = 0;
    try {
        val = std::stof(str);
    } catch (const std::exception& e) {
        LOG_FATAL("[Parameters][GetInt]cannot convert \"&\" to a float", str);
    }
    return val;
}

vec2i Parameters::GetVec2i(const std::string& name, vec2i fallback) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return fallback;
    std::string str = iter->second;
    if (str == "") {
        LOG_WARNING("[Parameters][GetVec2i]& do not have a value", name.c_str());
        return fallback;
    }

    vec2i v;
    StrToInts(str, &v[0], 2);
    return v;
}
vec3i Parameters::GetVec3i(const std::string& name, vec3i fallback) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return fallback;
    std::string str = iter->second;
    if (str == "") {
        LOG_WARNING("[Parameters][GetVec3i]& do not have a value", name.c_str());
        return fallback;
    }

    vec3i v;
    StrToInts(str, &v[0], 3);
    return v;
}
vec4i Parameters::GetVec4i(const std::string& name, vec4i fallback) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return fallback;
    std::string str = iter->second;
    if (str == "") {
        LOG_WARNING("[Parameters][GetVec4i]& do not have a value", name.c_str());
        return fallback;
    }

    vec4i v;
    StrToInts(str, &v[0], 4);
    return v;
}

vec2 Parameters::GetVec2(const std::string& name, vec2 fallback) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return fallback;
    std::string str = iter->second;
    if (str == "") {
        LOG_WARNING("[Parameters][GetVec2]& do not have a value", name.c_str());
        return fallback;
    }

    vec2 v;
    StrToFloats(str, &v[0], 2);
    return v;
}
vec3 Parameters::GetVec3(const std::string& name, vec3 fallback) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return fallback;
    std::string str = iter->second;
    if (str == "") {
        LOG_WARNING("[Parameters][GetVec3]& do not have a value", name.c_str());
        return fallback;
    }

    vec3 v;
    StrToFloats(str, &v[0], 3);
    return v;
}
vec4 Parameters::GetVec4(const std::string& name, vec4 fallback) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return fallback;
    std::string str = iter->second;
    if (str == "") {
        LOG_WARNING("[Parameters][GetVec4]& do not have a value", name.c_str());
        return fallback;
    }

    vec4 v;
    StrToFloats(str, &v[0], 4);
    return v;
}

std::string Parameters::GetString(const std::string& name, const std::string& fallback) const {
    auto iter = values.find(name);
    if (iter == values.end()) {
        if (name == "type" && fallback == "")
            return GetString("@");
        return fallback;
    }
    std::string str = iter->second;

    return str;
}

}  // namespace pine