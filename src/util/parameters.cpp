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
    auto& ss = subset[name];

    if (HasValue(name) && ss.size() == 0)
        AddSubset(name).Set("@", GetString(name));

    if (ss.size() == 0) {
        ss.resize(1);
        return ss[0];
    }
    if (ss.size() != 1) {
        LOG_WARNING("[Parameters][GetSubset]Find & subset with name \"&\", returns the last one",
                    ss.size(), name);
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
    return subset.find(name) != subset.end() || HasValue(name);
}

std::optional<bool> Parameters::TryGetBool(const std::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return std::nullopt;
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
std::optional<int> Parameters::TryGetInt(const std::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return std::nullopt;
    std::string str = iter->second;

    int val = 0;
    try {
        val = std::stoi(str);
    } catch (const std::exception& e) {
        LOG_FATAL("[Parameters][GetInt]cannot convert \"&\" to an int", str);
    }
    return val;
}
std::optional<float> Parameters::TryGetFloat(const std::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return std::nullopt;
    std::string str = iter->second;

    float val = 0;
    try {
        val = std::stof(str);
    } catch (const std::exception& e) {
        LOG_FATAL("[Parameters][GetInt]cannot convert \"&\" to a float", str);
    }
    return val;
}

std::optional<vec2i> Parameters::TryGetVec2i(const std::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return std::nullopt;
    std::string str = iter->second;

    vec2i v;
    StrToInts(str, &v[0], 2);
    return v;
}
std::optional<vec3i> Parameters::TryGetVec3i(const std::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return std::nullopt;
    std::string str = iter->second;

    vec3i v;
    StrToInts(str, &v[0], 3);
    return v;
}
std::optional<vec4i> Parameters::TryGetVec4i(const std::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return std::nullopt;
    std::string str = iter->second;

    vec4i v;
    StrToInts(str, &v[0], 4);
    return v;
}

std::optional<vec2> Parameters::TryGetVec2(const std::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return std::nullopt;
    std::string str = iter->second;

    vec2 v;
    StrToFloats(str, &v[0], 2);
    return v;
}
std::optional<vec3> Parameters::TryGetVec3(const std::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return std::nullopt;
    std::string str = iter->second;

    vec3 v;
    StrToFloats(str, &v[0], 3);
    return v;
}
std::optional<vec4> Parameters::TryGetVec4(const std::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return std::nullopt;
    std::string str = iter->second;

    vec4 v;
    StrToFloats(str, &v[0], 4);
    return v;
}

std::optional<std::string> Parameters::TryGetString(const std::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end()) {
        if (name == "type" && HasValue("@"))
            return GetString("@");
        return std::nullopt;
    }
    std::string str = iter->second;

    return str;
}

}  // namespace pine