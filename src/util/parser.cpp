#include <core/scene.h>
#include <util/parser.h>
#include <util/fileio.h>
#include <util/profiler.h>
#include <util/log.h>

#include <locale>
#include <algorithm>

namespace pine {

template <typename F>
pstd::optional<size_t> FirstOfF(pstd::string_view str, F&& f) {
    for (size_t i = 0; i < str.size(); i++)
        if (f(str[i]))
            return i;
    return pstd::nullopt;
}

static bool IsLetter(char s) {
    return (s >= 'A' && s <= 'z') || (s >= '0' && s <= '9') || s == '_';
}

static pstd::optional<size_t> FirstLetter(pstd::string_view str) {
    return FirstOfF(str, IsLetter);
}

static pstd::optional<size_t> FirstOf(pstd::string_view str, const pstd::vector<char>& chars) {
    return FirstOfF(str, [&](char s) {
        for (char c : chars)
            if (s == c)
                return true;
        return false;
    });
}

static void EscapeSpace(pstd::string_view& str) {
    while (str.size() && std::isspace(str[0])) {
        str = str.substr(1);
        if (str.substr(0, 2) == "//") {
            while (str[0] != '\n' && str[0] != '\r')
                str = str.substr(1);
            str = str.substr(1);
        }
    }
}

// TODO
static Parameters ParseBlock(pstd::string_view& block, int depth = 0) {
    Parameters params;

    while (true) {
        EscapeSpace(block);
        pstd::optional<size_t> keyBegin = FirstLetter(block);
        if (!keyBegin)
            break;

        pstd::optional<size_t> seperator = FirstOf(block, {':', '=', '{'});
        CHECK(seperator);
        if (auto blockEnd = FirstOf(block, {'}'}))
            if (*seperator > *blockEnd)
                break;

        auto keyEnd = FirstOfF(block, [](char s) { return !IsLetter(s); });
        CHECK(keyEnd);
        pstd::string key = (pstd::string)block.substr(0, *keyEnd);
        pstd::string name = (pstd::string)block.substr(*keyEnd, *seperator - *keyEnd);

        // TODO
        std::string name_std = std::string(pstd::begin(name), pstd::end(name));
        name_std.erase(
            std::remove_if(begin(name_std), end(name_std), [](char s) { return !IsLetter(s); }),
            end(name_std));
        name.assign(pstd::begin(name_std), pstd::end(name_std));
        //

        block = block.substr(*seperator);

        bool isTyped = false;
        pstd::string value;
        if (block[0] != '{') {
            isTyped = true;
            block = block.substr(1);
            EscapeSpace(block);
            pstd::optional<size_t> valueEnd =
                FirstOfF(block, [](char s) { return s == '\n' || s == '\r' || s == '{'; });
            CHECK(valueEnd);

            value = (pstd::string)block.substr(0, *valueEnd);
            params.Set(key, value);
            block = block.substr(*valueEnd);
        }
        EscapeSpace(block);

        if (block[0] == '{') {
            block = block.substr(1);
            auto& subset = params.AddSubset(key) = ParseBlock(block, depth + 1);
            if (isTyped)
                subset.Set("@", value);
            if (name != "" && !subset.HasValue("name"))
                subset.Set("name", name);
        }

    }

    if (auto blockEnd = FirstOf(block, {'}'}))
        block = block.substr(*blockEnd + 1);

    return params;
}

Parameters Parse(pstd::string_view raw) {
    Profiler _("Parse");
    Timer timer;

    Parameters params = ParseBlock(raw);

    return params;
}

}  // namespace pine