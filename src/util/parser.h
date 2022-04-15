#ifndef PINE_UTIL_PARSER_H
#define PINE_UTIL_PARSER_H

#include <core/defines.h>
#include <util/parameters.h>

#include <string_view>

namespace pine {

Parameters Parse(pstd::string_view raw);

}  // namespace pine

#endif  // PINE_UTIL_PARSER_H