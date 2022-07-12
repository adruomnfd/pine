#ifndef PINE_UTIL_OBJLOADER_H

#include <core/defines.h>

#include <pstd/string.h>

namespace pine {

TriangleMesh LoadObj(pstd::string_view string_view);

}  // namespace pine

#endif  // PINE_UTIL_OBJLODER_H