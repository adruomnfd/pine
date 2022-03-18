#include <core/scene.h>
#include <util/parser.h>
#include <util/fileio.h>
#include <util/profiler.h>
#include <util/log.h>

namespace pine {

size_t findFirstLetter(std::string_view str) {
    size_t pos = str.npos;
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (i == str.size())
            break;
        if (str[i] >= 'A' && str[i] <= 'z') {
            pos = i;
            break;
        }
    }
    return pos;
}

Parameters ParseBlock(std::string_view block, size_t& recursiveBlockEnd) {
    Parameters params;
    std::string_view raw = block;
    size_t blockEnd = 0;

    while (true) {
        size_t keyBegin = findFirstLetter(block);
        if (keyBegin == block.npos || keyBegin >= block.find_first_of('}'))
            break;

        size_t seperator = block.find_first_of(':');

        size_t subBlockStart = block.find_first_of('{');
        if (subBlockStart < block.find_first_of('}') && subBlockStart < seperator) {
            size_t subTypeBegin = findFirstLetter(block);
            if (subTypeBegin == raw.npos) {
                LOG_WARNING("[Parser]Missing type name after \"&\"", (std::string)block);
                break;
            }

            std::string_view subType = block.substr(subTypeBegin, subBlockStart - subTypeBegin);

            size_t subBlockEnd = subBlockStart + 1;
            params[(std::string)subType] = ParseBlock(block.substr(subBlockStart + 1), subBlockEnd);

            blockEnd += subBlockEnd + 1;
            block = raw.substr(blockEnd);
            continue;
        }

        size_t valueBegin = block.substr(seperator + 1).find_first_not_of(' ');
        if (valueBegin == block.npos) {
            LOG_WARNING("[Parser]Missing value after \"&\"", (std::string)block);
            break;
        }
        valueBegin += seperator + 1;

        size_t valueEnd = block.substr(valueBegin + 1).find_first_of('\n');
        if (valueEnd == block.npos) {
            LOG_WARNING("[Parser]Missing new line after \"&\"", (std::string)block);
            break;
        }
        valueEnd += valueBegin + 1;

        params.Set((std::string)block.substr(keyBegin, seperator - keyBegin),
                   (std::string)block.substr(valueBegin, valueEnd - valueBegin));

        blockEnd += valueEnd + 1;
        block = raw.substr(blockEnd);
    }

    blockEnd += block.find_first_of('}') + 1;
    recursiveBlockEnd += blockEnd;

    return params;
}

Parameters Parse(std::string_view raw) {
    Profiler _("Parse");
    LOG_VERBOSE("[FileIO]Parsing parameters");
    Timer timer;

    Parameters parameters;

    while (true) {
        size_t typeBegin = findFirstLetter(raw);
        if (typeBegin == raw.npos)
            break;

        size_t blockStart = raw.find_first_of('{');
        if (blockStart == raw.npos) {
            LOG_WARNING("[Parser]Missing \"{\" after \"&\"", (std::string)raw.substr(typeBegin));
            break;
        }
        size_t blockEnd = blockStart + 1;

        std::string type = (std::string)raw.substr(typeBegin, blockStart - typeBegin);
        Parameters subParams = ParseBlock(raw.substr(blockStart + 1), blockEnd);
        parameters[type][subParams.GetString("name", "singleton")] = subParams;

        raw = raw.substr(blockEnd);
    }
    LOG_VERBOSE("[Parser]Parameter parsed in & ms", timer.ElapsedMs());
    return parameters;
}

}  // namespace pine