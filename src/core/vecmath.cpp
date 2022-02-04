#include <core/vecmath.h>

namespace pine {

template struct Vector2<uint8_t>;
template struct Vector3<uint8_t>;
template struct Vector4<uint8_t>;
template struct Vector3<uint32_t>;
template struct Vector2<int>;
template struct Vector3<int>;
template struct Vector4<int>;
template struct Vector2<float>;
template struct Vector3<float>;
template struct Vector4<float>;
template struct Matrix3<float>;
template struct Matrix4<float>;

}  // namespace pine