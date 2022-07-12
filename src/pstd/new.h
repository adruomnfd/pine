#ifndef PINE_STD_NEW_H
#define PINE_STD_NEW_H

#if 1

#include <new>

#else

#ifndef _NEW
#define _NEW
#include <pstd/stdint.h>
inline void* operator new(pstd::size_t, void* p) {
    return p;
}
inline void* operator new[](pstd::size_t, void* p) {
    return p;
}
inline void operator delete(void*, void*) {
}
inline void operator delete[](void*, void*) {
}
#endif

#endif

#endif  // PINE_STD_NEW_H