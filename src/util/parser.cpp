#include <core/scene.h>
#include <util/parser.h>
#include <util/fileio.h>
#include <util/profiler.h>
#include <util/log.h>

#include <locale>
#include <algorithm>

namespace pine {

template <typename F>
std::optional<size_t> FirstOfF(std::string_view str, F&& f) {
    for (size_t i = 0; i < str.size(); i++)
        if (f(str[i]))
            return i;
    return std::nullopt;
}

bool IsLetter(char s) {
    return (s >= 'A' && s <= 'z') || (s >= '0' && s <= '9') || s == '_';
}

std::optional<size_t> FirstLetter(std::string_view str) {
    return FirstOfF(str, IsLetter);
}

std::optional<size_t> FirstOf(std::string_view str, const std::vector<char>& chars) {
    return FirstOfF(str, [&](char s) {
        for (char c : chars)
            if (s == c)
                return true;
        return false;
    });
}

void EscapeSpace(std::string_view& str) {
    while (std::isspace(str[0])) {
        str = str.substr(1);
        if (str.substr(0, 2) == "//") {
            while (str[0] != '\n' && str[0] != '\r')
                str = str.substr(1);
            str = str.substr(1);
        }
    }
}

Parameters ParseBlock(std::string_view& block, int depth = 0) {
    Parameters params;

    while (true) {
        EscapeSpace(block);
        std::optional<size_t> keyBegin = FirstLetter(block);
        if (!keyBegin)
            break;

        std::optional<size_t> seperator = FirstOf(block, {':', '=', '{'});
        CHECK(seperator);
        if (auto blockEnd = FirstOf(block, {'}'}))
            if (*seperator > *blockEnd)
                break;

        auto keyEnd = FirstOfF(block, [](char s) { return !IsLetter(s); });
        CHECK(keyEnd);
        std::string key = (std::string)block.substr(0, *keyEnd);
        std::string name = (std::string)block.substr(*keyEnd, *seperator - *keyEnd);
        name.erase(std::remove_if(begin(name), end(name), [](char s) { return !IsLetter(s); }),
                   end(name));
        block = block.substr(*seperator);

        bool isTyped = false;
        std::string value;
        if (block[0] != '{') {
            isTyped = true;
            block = block.substr(1);
            EscapeSpace(block);
            std::optional<size_t> valueEnd =
                FirstOfF(block, [](char s) { return s == '\n' || s == '\r' || s == '{'; });
            CHECK(valueEnd);

            value = (std::string)block.substr(0, *valueEnd);
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

Parameters Parse(std::string_view raw) {
    Profiler _("Parse");
    LOG_VERBOSE("[FileIO]Parsing parameters");
    Timer timer;

    Parameters parameters = ParseBlock(raw);

    LOG_VERBOSE("[Parser]Parameter parsed in & ms", timer.ElapsedMs());
    return parameters;
}

}  // namespace pine