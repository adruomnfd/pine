#include <util/parameters.h>
#include <util/log.h>

#include <sstream>

namespace pine {

void Parameters::Summarize(int indent) const {
    // for (const auto& v : values)
    //     LOG("& &: &", Format(indent), " ", v.first.c_str(), v.second.c_str());
    // for (const auto& s : subset) {
    //     for (auto& ss : s.second) {
    //         LOG("& &", Format(indent), " ", s.first);
    //         ss.Summarize(indent + 2);
    //     }
    // }

    for (const auto& v : values)
        LOG(v.first.c_str(), ": ", v.second.c_str());
    for (const auto& s : subset) {
        for (auto& ss : s.second) {
            LOG(s.first);
            ss.Summarize(indent + 2);
        }
    }
}

const pstd::vector<Parameters>& Parameters::GetAll(pstd::string name) const {
    return subset[name];
}
Parameters& Parameters::AddSubset(pstd::string name) {
    auto& sub = subset[name];
    sub.resize(sub.size() + 1);
    return sub.back();
}

Parameters& Parameters::operator[](pstd::string name) {
    auto& ss = subset[name];

    if (HasValue(name) && ss.size() == 0)
        AddSubset(name).Set("@", GetString(name));

    if (ss.size() == 0) {
        ss.resize(1);
        return ss[0];
    }
    if (ss.size() != 1) {
        LOG_WARNING("[Parameters][GetSubset]Find & subsets with name \"&\", returns the last one",
                    ss.size(), name);
    }
    return subset[name].back();
}
const Parameters& Parameters::operator[](pstd::string name) const {
    return (*const_cast<Parameters*>(this))[name];
}

bool Parameters::HasValue(const pstd::string& name) const {
    return values.find(name) != values.end();
}

bool Parameters::HasSubset(const pstd::string& name) const {
    return subset.find(name) != subset.end() || HasValue(name);
}

pstd::optional<bool> Parameters::TryGetBool(const pstd::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return pstd::nullopt;
    pstd::string str = iter->second;
    for (size_t i = 0; i < str.size(); i++)
        if ('A' <= str[i] && str[i] <= 'Z')
            str[i] += 'a' - 'A';

    if (str == "true")
        return true;
    else if (str == "false")
        return false;
    else
        return (bool)pstd::stoi(str);
}
pstd::optional<int> Parameters::TryGetInt(const pstd::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return pstd::nullopt;
    pstd::string str = iter->second;

    int val = pstd::stoi(str);

    return val;
}
pstd::optional<float> Parameters::TryGetFloat(const pstd::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return pstd::nullopt;
    pstd::string str = iter->second;

    float val = pstd::stof(str);
    return val;
}

pstd::optional<vec2i> Parameters::TryGetVec2i(const pstd::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return pstd::nullopt;
    pstd::string str = iter->second;

    vec2i v;
    pstd::stois(str, &v[0], 2);
    return v;
}
pstd::optional<vec3i> Parameters::TryGetVec3i(const pstd::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return pstd::nullopt;
    pstd::string str = iter->second;

    vec3i v;
    pstd::stois(str, &v[0], 3);
    return v;
}
pstd::optional<vec4i> Parameters::TryGetVec4i(const pstd::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return pstd::nullopt;
    pstd::string str = iter->second;

    vec4i v;
    pstd::stois(str, &v[0], 4);
    return v;
}

pstd::optional<vec2> Parameters::TryGetVec2(const pstd::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return pstd::nullopt;
    pstd::string str = iter->second;

    vec2 v;
    pstd::stofs(str, &v[0], 2);
    return v;
}
pstd::optional<vec3> Parameters::TryGetVec3(const pstd::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return pstd::nullopt;
    pstd::string str = iter->second;

    vec3 v;
    pstd::stofs(str, &v[0], 3);
    return v;
}
pstd::optional<vec4> Parameters::TryGetVec4(const pstd::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end())
        return pstd::nullopt;
    pstd::string str = iter->second;

    vec4 v;
    pstd::stofs(str, &v[0], 4);
    return v;
}

pstd::optional<pstd::string> Parameters::TryGetString(const pstd::string& name) const {
    auto iter = values.find(name);
    if (iter == values.end()) {
        if (name == "type" && HasValue("@"))
            return GetString("@");
        return pstd::nullopt;
    }
    pstd::string str = iter->second;

    return str;
}

}  // namespace pine