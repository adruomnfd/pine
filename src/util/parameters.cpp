#include <util/parameters.h>
#include <util/log.h>

namespace pine {

void Parameters::Summarize(int indent) const {
    size_t maxWidth = 0;
    for (const auto& v : values)
        maxWidth = std::max(v.first.size(), maxWidth);
    for (const auto& v : values)
        LOG_VERBOSE("[Parameters]& &<: &", Format(indent), " ", Format(maxWidth), v.first.c_str(),
                    v.second.c_str());
    for (const auto& s : subset) {
        LOG("[Parameters]&", s.first);
        s.second.Summarize(indent + 2);
    }
}

void Parameters::Override(const Parameters& it) {
    for (const auto& v : it.values)
        values[v.first] = v.second;
}

bool Parameters::Has(const std::string& name) const {
    return values.find(name) != values.end();
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

    return std::stoi(str);
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

    if (str == "Pi")
        return Pi;
    if (str == "2Pi")
        return 2 * Pi;

    return std::stof(str);
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
    if (iter == values.end())
        return fallback;
    std::string str = iter->second;

    return str;
}

}  // namespace pine