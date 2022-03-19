#ifndef PINE_CORE_VECMATH_H
#define PINE_CORE_VECMATH_H

#include <core/math.h>
#include <util/string.h>

namespace pine {

template <typename T>
struct Vector2;
template <typename T>
struct Vector3;
template <typename T>
struct Vector4;

template <typename T>
struct Matrix3;

template <typename T>
struct Matrix4;

template <typename T>
struct Vector2 {
    Vector2() = default;
    template <typename U>
    explicit Vector2(U v) : x((T)v), y((T)v) {
    }
    template <typename U>
    Vector2(U x, U y) : x((T)x), y((T)y) {
    }
    template <typename U>
    Vector2(const Vector2<U> &v) : x((T)v.x), y((T)v.y) {
    }
    template <typename U>
    Vector2(Vector3<U> v) : x((T)v.x), y((T)v.y) {
    }
    template <typename U>
    Vector2(Vector4<U> v) : x((T)v.x), y((T)v.y) {
    }

    template <typename U>
    Vector2 &operator+=(Vector2<U> rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    template <typename U>
    Vector2 &operator-=(Vector2<U> rhs) {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }
    template <typename U>
    Vector2 &operator*=(Vector2<U> rhs) {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }
    template <typename U>
    Vector2 &operator/=(Vector2<U> rhs) {
        x /= rhs.x;
        y /= rhs.y;
        return *this;
    }
    template <typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
    Vector2 &operator*=(U rhs) {
        x *= rhs;
        y *= rhs;
        return *this;
    }
    template <typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
    Vector2 &operator/=(U rhs) {
        x /= rhs;
        y /= rhs;
        return *this;
    }

    template <typename U>
    friend Vector2<decltype(T{} + U{})> operator+(Vector2<T> lhs, Vector2<U> rhs) {
        return {lhs.x + rhs.x, lhs.y + rhs.y};
    }
    template <typename U>
    friend Vector2<decltype(T{} - U{})> operator-(Vector2<T> lhs, Vector2<U> rhs) {
        return {lhs.x - rhs.x, lhs.y - rhs.y};
    }
    template <typename U>
    friend Vector2<decltype(T{} * U{})> operator*(Vector2<T> lhs, Vector2<U> rhs) {
        return {lhs.x * rhs.x, lhs.y * rhs.y};
    }
    template <typename U>
    friend Vector2<decltype(T{} / U{})> operator/(Vector2<T> lhs, Vector2<U> rhs) {
        return {lhs.x / rhs.x, lhs.y / rhs.y};
    }
    template <typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
    friend Vector2<decltype(T{} * U{})> operator*(Vector2<T> lhs, U rhs) {
        return {lhs.x * rhs, lhs.y * rhs};
    }
    template <typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
    friend Vector2<decltype(T{} / U{})> operator/(Vector2<T> lhs, U rhs) {
        return {lhs.x / rhs, lhs.y / rhs};
    }
    template <typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
    friend Vector2<decltype(U{} * T{})> operator*(U lhs, Vector2<T> rhs) {
        return {lhs * rhs.x, lhs * rhs.y};
    }
    template <typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
    friend Vector2<decltype(U{} / T{})> operator/(U lhs, Vector2<T> rhs) {
        return {lhs / rhs.x, lhs / rhs.y};
    }
    template <typename U>
    friend bool operator==(Vector2<T> lhs, Vector2<U> rhs) {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
    template <typename U>
    friend bool operator!=(Vector2<T> lhs, Vector2<U> rhs) {
        return lhs.x != rhs.x || lhs.y != rhs.y;
    }

    Vector2 operator-() const {
        return {-x, -y};
    }

    T &operator[](int i) {
        return (&x)[i];
    }
    const T &operator[](int i) const {
        return (&x)[i];
    }

    Fstring Formatting(Format fmt) const {
        return Fstring(fmt, "[&, &]", x, y);
    }
    std::string ToString() const {
        return pine::ToString('[', x, ", ", y, ']');
    }

    PINE_ARCHIVE(x, y)

    T x{}, y{};
};

template <typename T>
struct Vector3 {
    Vector3() = default;
    template <typename U>
    explicit Vector3(U v) : x((T)v), y((T)v), z((T)v) {
    }
    template <typename U>
    Vector3(U x, U y, U z) : x((T)x), y((T)y), z((T)z) {
    }
    template <typename U>
    Vector3(Vector2<U> xy) : x((T)xy.x), y((T)xy.y), z((T)0) {
    }
    template <typename U>
    Vector3(const Vector3<U> &v) : x((T)v.x), y((T)v.y), z((T)v.z) {
    }
    template <typename U>
    Vector3(Vector4<U> v) : x((T)v.x), y((T)v.y), z((T)v.z) {
    }
    template <typename U>
    Vector3(Vector2<U> xy, U z) : x((T)xy.x), y((T)xy.y), z((T)z) {
    }
    template <typename U>
    Vector3(U x, Vector2<U> yz) : x((T)x), y((T)yz.x), z((T)yz.y) {
    }

    template <typename U>
    Vector3 &operator+=(Vector3<U> rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }
    template <typename U>
    Vector3 &operator-=(Vector3<U> rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }
    template <typename U>
    Vector3 &operator*=(Vector3<U> rhs) {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }
    template <typename U>
    Vector3 &operator/=(Vector3<U> rhs) {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        return *this;
    }
    template <typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
    Vector3 &operator*=(U rhs) {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }
    template <typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
    Vector3 &operator/=(U rhs) {
        x /= rhs;
        y /= rhs;
        z /= rhs;
        return *this;
    }

    template <typename U>
    friend Vector3<decltype(T{} + U{})> operator+(Vector3<T> lhs, Vector3<U> rhs) {
        return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
    }
    template <typename U>
    friend Vector3<decltype(T{} - U{})> operator-(Vector3<T> lhs, Vector3<U> rhs) {
        return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
    }
    template <typename U>
    friend Vector3<decltype(T{} * U{})> operator*(Vector3<T> lhs, Vector3<U> rhs) {
        return {lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z};
    }
    template <typename U>
    friend Vector3<decltype(T{} / U{})> operator/(Vector3<T> lhs, Vector3<U> rhs) {
        return {lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z};
    }
    template <typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
    friend Vector3<decltype(T{} * U{})> operator*(Vector3<T> lhs, U rhs) {
        return {lhs.x * rhs, lhs.y * rhs, lhs.z * rhs};
    }
    template <typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
    friend Vector3<decltype(T{} / U{})> operator/(Vector3<T> lhs, U rhs) {
        return {lhs.x / rhs, lhs.y / rhs, lhs.z / rhs};
    }
    template <typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
    friend Vector3<decltype(U{} * T{})> operator*(U lhs, Vector3<T> rhs) {
        return {lhs * rhs.x, lhs * rhs.y, lhs * rhs.z};
    }
    template <typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
    friend Vector3<decltype(U{} / T{})> operator/(U lhs, Vector3<T> rhs) {
        return {lhs / rhs.x, lhs / rhs.y, lhs / rhs.z};
    }
    template <typename U>
    friend bool operator==(Vector3<T> lhs, Vector3<U> rhs) {
        return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
    }
    template <typename U>
    friend bool operator!=(Vector3<T> lhs, Vector3<U> rhs) {
        return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z;
    }

    Vector3 operator-() const {
        return {-x, -y, -z};
    }

    T &operator[](int i) {
        return (&x)[i];
    }
    const T &operator[](int i) const {
        return (&x)[i];
    }

    Fstring Formatting(Format fmt) const {
        return Fstring(fmt, "[&, &, &]", x, y, z);
    }
    std::string ToString() const {
        return pine::ToString('[', x, ", ", y, ", ", z, ']');
    }

    PINE_ARCHIVE(x, y, z)

    T x{}, y{}, z{};
};

template <typename T>
struct Vector4 {
    Vector4() = default;
    template <typename U>
    explicit Vector4(U v) : x((T)v), y((T)v), z((T)v), w((T)v) {
    }
    template <typename U>
    Vector4(U x, U y, U z, U w) : x((T)x), y((T)y), z((T)z), w((T)w) {
    }
    template <typename U>
    Vector4(Vector2<U> v) : x((T)v.x), y((T)v.y), z((T)0), w((T)0) {
    }
    template <typename U>
    Vector4(Vector3<U> v) : x((T)v.x), y((T)v.y), z((T)v.z), w((T)0) {
    }
    template <typename U>
    Vector4(const Vector4<U> &v) : x((T)v.x), y((T)v.y), z((T)v.z), w((T)v.w) {
    }
    template <typename U>
    Vector4(Vector2<U> xy, Vector2<U> zw) : x((T)xy.x), y((T)xy.y), z((T)zw.x), w((T)zw.y) {
    }
    template <typename U>
    Vector4(Vector2<U> xy, U z, U w) : x((T)xy.x), y((T)xy.y), z((T)z), w((T)w) {
    }
    template <typename U>
    Vector4(Vector3<U> xyz, U w) : x((T)xyz.x), y((T)xyz.y), z((T)xyz.z), w((T)w) {
    }
    template <typename U>
    Vector4(U x, Vector3<U> yzw) : x((T)x), y((T)yzw.x), z((T)yzw.y), w((T)yzw.z) {
    }

    template <typename U>
    Vector4 &operator+=(Vector4<U> rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }
    template <typename U>
    Vector4 &operator-=(Vector4<U> rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }
    template <typename U>
    Vector4 &operator*=(Vector4<U> rhs) {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        w *= rhs.w;
        return *this;
    }
    template <typename U>
    Vector4 &operator/=(Vector4<U> rhs) {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        w /= rhs.w;
        return *this;
    }

    template <typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
    Vector4 &operator*=(U rhs) {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        w *= rhs;
        return *this;
    }
    template <typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
    Vector4 &operator/=(U rhs) {
        x /= rhs;
        y /= rhs;
        z /= rhs;
        w /= rhs;
        return *this;
    }

    template <typename U>
    friend Vector4<decltype(T{} + U{})> operator+(Vector4<T> lhs, Vector4<U> rhs) {
        return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w};
    }
    template <typename U>
    friend Vector4<decltype(T{} - U{})> operator-(Vector4<T> lhs, Vector4<U> rhs) {
        return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w};
    }
    template <typename U>
    friend Vector4<decltype(T{} * U{})> operator*(Vector4<T> lhs, Vector4<U> rhs) {
        return {lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w};
    }
    template <typename U>
    friend Vector4<decltype(T{} / U{})> operator/(Vector4<T> lhs, Vector4<U> rhs) {
        return {lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w};
    }
    template <typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
    friend Vector4<decltype(T{} * U{})> operator*(Vector4<T> lhs, U rhs) {
        return {lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs};
    }
    template <typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
    friend Vector4<decltype(T{} / U{})> operator/(Vector4<T> lhs, U rhs) {
        return {lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs};
    }
    template <typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
    friend Vector4<decltype(U{} * T{})> operator*(U lhs, Vector4<T> rhs) {
        return {lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w};
    }
    template <typename U, typename = typename std::enable_if<std::is_arithmetic<U>::value>::type>
    friend Vector4<decltype(U{} / T{})> operator/(U lhs, Vector4<T> rhs) {
        return {lhs / rhs.x, lhs / rhs.y, lhs / rhs.z, lhs / rhs.w};
    }
    template <typename U>
    friend bool operator==(Vector4<T> lhs, Vector4<U> rhs) {
        return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
    }
    template <typename U>
    friend bool operator!=(Vector4<T> lhs, Vector4<U> rhs) {
        return lhs.x != rhs.x || lhs.y != rhs.y || lhs.z != rhs.z || lhs.w != rhs.w;
    }

    Vector4 operator-() const {
        return {-x, -y, -z, -w};
    }

    T &operator[](int i) {
        return (&x)[i];
    }
    const T &operator[](int i) const {
        return (&x)[i];
    }

    Fstring Formatting(Format fmt) const {
        return Fstring(fmt, "[&, &, &, &]", x, y, z, w);
    }

    PINE_ARCHIVE(x, y, z, w)

    T x{}, y{}, z{}, w{};
};

template <typename T>
struct Matrix2 {
    static Matrix2 Zero() {
        return Matrix2(0, 0, 0, 0);
    }

    static Matrix2 Identity() {
        return Matrix2(1, 0, 1, 0);
    }

    static bool IsIdentity(const Matrix2 &m) {
        return m == Identity();
    }

    static bool IsZero(const Matrix2 &m) {
        return m == Zero();
    }

    Matrix2() {
        *this = Identity();
    }

    Matrix2(T x0, T y0, T x1, T y1) : x(x0, x1), y(y0, y1) {
    }

    Matrix2(Vector2<T> x, Vector2<T> y) : x(x), y(y){};

    Matrix2 &operator+=(const Matrix2 &rhs) {
        for (int c = 0; c < N; c++)
            for (int r = 0; r < N; r++)
                (*this)[c][r] += rhs[c][r];
        return *this;
    }

    Matrix2 &operator-=(const Matrix2 &rhs) {
        for (int c = 0; c < N; c++)
            for (int r = 0; r < N; r++)
                (*this)[c][r] -= rhs[c][r];
        return *this;
    }

    Matrix2 &operator*=(T rhs) {
        for (int c = 0; c < N; c++)
            for (int r = 0; r < N; r++)
                (*this)[c][r] *= rhs;
        return *this;
    }

    Matrix2 &operator/=(T rhs) {
        for (int c = 0; c < N; c++)
            for (int r = 0; r < N; r++)
                (*this)[c][r] /= rhs;
        return *this;
    }

    friend Matrix2 operator*(Matrix2 lhs, T rhs) {
        return lhs *= rhs;
    }
    friend Matrix2 operator/(Matrix2 lhs, T rhs) {
        return lhs /= rhs;
    }

    friend Matrix2 operator*(const Matrix2 &lhs, const Matrix2 &rhs) {
        Matrix2 ret = Zero();
        for (int c = 0; c < N; c++)
            for (int r = 0; r < N; r++) {
                for (int i = 0; i < N; i++)
                    ret[c][r] += lhs[i][r] * rhs[c][i];
            }

        return ret;
    }

    Vector2<T> Row(int i) const {
        return {x[i], y[i]};
    }

    Vector2<T> &operator[](int i) {
        return (&x)[i];
    }
    const Vector2<T> &operator[](int i) const {
        return (&x)[i];
    }

    friend bool operator==(const Matrix2 &m0, const Matrix2 &m1) {
        return m0.x == m1.x && m0.y == m1.y;
    }
    friend bool operator!=(const Matrix2 &m0, const Matrix2 &m1) {
        return m0.x != m1.x || m0.y != m1.y;
    }

    Fstring Formatting(Format fmt) const {
        return Fstring(fmt, "\n[&,\n &,\n]", x, y);
    }

    PINE_ARCHIVE(x, y)

    static constexpr int N = 2;
    Vector2<T> x, y;
};

template <typename T>
struct Matrix3 {
    static Matrix3 Zero() {
        return Matrix3(0, 0, 0, 0, 0, 0, 0, 0, 0);
    }

    static Matrix3 Identity() {
        return Matrix3(1, 0, 0, 0, 1, 0, 0, 0, 1);
    }

    static bool IsIdentity(const Matrix3 &m) {
        return m == Identity();
    }

    static bool IsZero(const Matrix3 &m) {
        return m == Zero();
    }

    Matrix3() {
        *this = Identity();
    }

    Matrix3(T x0, T y0, T z0, T x1, T y1, T z1, T x2, T y2, T z2)
        : x(x0, x1, x2), y(y0, y1, y2), z(z0, z1, z2) {
    }

    Matrix3(Vector3<T> x, Vector3<T> y, Vector3<T> z) : x(x), y(y), z(z){};

    Matrix3 &operator+=(const Matrix3 &rhs) {
        for (int c = 0; c < N; c++)
            for (int r = 0; r < N; r++)
                (*this)[c][r] += rhs[c][r];
        return *this;
    }

    Matrix3 &operator-=(const Matrix3 &rhs) {
        for (int c = 0; c < N; c++)
            for (int r = 0; r < N; r++)
                (*this)[c][r] -= rhs[c][r];
        return *this;
    }

    Matrix3 &operator*=(T rhs) {
        for (int c = 0; c < N; c++)
            for (int r = 0; r < N; r++)
                (*this)[c][r] *= rhs;
        return *this;
    }

    Matrix3 &operator/=(T rhs) {
        for (int c = 0; c < N; c++)
            for (int r = 0; r < N; r++)
                (*this)[c][r] /= rhs;
        return *this;
    }

    friend Matrix3 operator*(Matrix3 lhs, T rhs) {
        return lhs *= rhs;
    }
    friend Matrix3 operator/(Matrix3 lhs, T rhs) {
        return lhs /= rhs;
    }

    friend Matrix3 operator*(const Matrix3 &lhs, const Matrix3 &rhs) {
        Matrix3 ret = Zero();
        for (int c = 0; c < N; c++)
            for (int r = 0; r < N; r++) {
                for (int i = 0; i < N; i++)
                    ret[c][r] += lhs[i][r] * rhs[c][i];
            }

        return ret;
    }

    Vector3<T> Row(int i) const {
        return {x[i], y[i], z[i]};
    }

    Vector3<T> &operator[](int i) {
        return (&x)[i];
    }
    const Vector3<T> &operator[](int i) const {
        return (&x)[i];
    }

    friend bool operator==(const Matrix3 &m0, const Matrix3 &m1) {
        return m0.x == m1.x && m0.y == m1.y && m0.z == m1.z;
    }
    friend bool operator!=(const Matrix3 &m0, const Matrix3 &m1) {
        return m0.x != m1.x || m0.y != m1.y || m0.z != m1.z;
    }

    Fstring Formatting(Format fmt) const {
        return Fstring(fmt, "\n[&,\n &,\n &]", x, y, z);
    }

    PINE_ARCHIVE(x, y, z)

    static constexpr int N = 3;
    Vector3<T> x, y, z;
};

template <typename T>
struct Matrix4 {
    static Matrix4 Zero() {
        return Matrix4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }

    static Matrix4 Identity() {
        return Matrix4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    }

    static bool IsIdentity(const Matrix4 &m) {
        return m == Identity();
    }

    static bool IsZero(const Matrix4 &m) {
        return m == Zero();
    }

    Matrix4() {
        *this = Identity();
    }

    Matrix4(T x0, T y0, T z0, T w0, T x1, T y1, T z1, T w1, T x2, T y2, T z2, T w2, T x3, T y3,
            T z3, T w3)
        : x(x0, x1, x2, x3), y(y0, y1, y2, y3), z(z0, z1, z2, z3), w(w0, w1, w2, w3) {
    }

    Matrix4(Vector4<T> x, Vector4<T> y, Vector4<T> z, Vector4<T> w) : x(x), y(y), z(z), w(w){};

    operator Matrix3<T>() const {
        return Matrix3<T>(x, y, z);
    }

    Matrix4 &operator+=(const Matrix4 &rhs) {
        for (int c = 0; c < N; c++)
            for (int r = 0; r < N; r++)
                (*this)[c][r] += rhs[c][r];
        return *this;
    }

    Matrix4 &operator-=(const Matrix4 &rhs) {
        for (int c = 0; c < N; c++)
            for (int r = 0; r < N; r++)
                (*this)[c][r] -= rhs[c][r];
        return *this;
    }

    Matrix4 &operator*=(T rhs) {
        for (int c = 0; c < N; c++)
            for (int r = 0; r < N; r++)
                (*this)[c][r] *= rhs;
        return *this;
    }

    Matrix4 &operator/=(T rhs) {
        for (int c = 0; c < N; c++)
            for (int r = 0; r < N; r++)
                (*this)[c][r] /= rhs;
        return *this;
    }

    friend Matrix4 operator*(Matrix4 lhs, T rhs) {
        return lhs *= rhs;
    }
    friend Matrix4 operator/(Matrix4 lhs, T rhs) {
        return lhs /= rhs;
    }

    friend Matrix4 operator*(const Matrix4 &lhs, const Matrix4 &rhs) {
        Matrix4 ret = Zero();
        for (int c = 0; c < N; c++)
            for (int r = 0; r < N; r++) {
                for (int i = 0; i < N; i++)
                    ret[c][r] += lhs[i][r] * rhs[c][i];
            }
        return ret;
    }

    Vector4<T> Row(int i) const {
        return {x[i], y[i], z[i], w[i]};
    }

    Vector4<T> &operator[](int i) {
        return (&x)[i];
    }
    const Vector4<T> &operator[](int i) const {
        return (&x)[i];
    }

    friend bool operator==(const Matrix4 &m0, const Matrix4 &m1) {
        return m0.x == m1.x && m0.y == m1.y && m0.z == m1.z && m0.w == m1.w;
    }
    friend bool operator!=(const Matrix4 &m0, const Matrix4 &m1) {
        return m0.x != m1.x || m0.y != m1.y || m0.z != m1.z || m0.w != m1.w;
    }

    Fstring Formatting(Format fmt) const {
        return Fstring(fmt, "\n[&,\n &,\n &,\n &]", x, y, z, w);
    }

    PINE_ARCHIVE(x, y, z, w)

    static constexpr int N = 4;
    Vector4<T> x, y, z, w;
};

typedef Vector2<uint8_t> vec2u8;
typedef Vector3<uint8_t> vec3u8;
typedef Vector4<uint8_t> vec4u8;
typedef Vector2<uint32_t> vec2u32;
typedef Vector3<uint32_t> vec3u32;
typedef Vector4<uint32_t> vec4u32;
typedef Vector2<int> vec2i;
typedef Vector3<int> vec3i;
typedef Vector4<int> vec4i;
typedef Vector2<float> vec2;
typedef Vector3<float> vec3;
typedef Vector4<float> vec4;
typedef Matrix2<float> mat2;
typedef Matrix3<float> mat3;
typedef Matrix4<float> mat4;

template <typename T>
inline Vector2<T> operator*(const Matrix2<T> &m, const Vector2<T> &v) {
    return m.x * v.x + m.y * v.y;
}

template <typename T>
inline Vector3<T> operator*(const Matrix3<T> &m, const Vector3<T> &v) {
    return m.x * v.x + m.y * v.y + m.z * v.z;
}

template <typename T>
inline Vector4<T> operator*(const Matrix4<T> &m, const Vector4<T> &v) {
    return m.x * v.x + m.y * v.y + m.z * v.z + m.w * v.w;
}

template <typename T>
inline Vector2<T> ToVec2(const T *src) {
    Vector2<T> v;
    for (int i = 0; i < 2; i++)
        v[i] = src[i];
    return v;
}

template <typename T>
inline Vector3<T> ToVec3(const T *src) {
    Vector3<T> v;
    for (int i = 0; i < 3; i++)
        v[i] = src[i];
    return v;
}

template <typename T>
inline Vector4<T> ToVec4(const T *src) {
    Vector4<T> v;
    for (int i = 0; i < 4; i++)
        v[i] = src[i];
    return v;
}

template <typename T>
inline Matrix3<T> ToMat3(const T *src) {
    Matrix3<T> m;
    for (int c = 0; c < 3; c++)
        for (int r = 0; r < 3; r++)
            m[c][r] = src[c * 3 + r];
    return m;
}

template <typename T>
inline Matrix4<T> ToMat4(const T *src) {
    Matrix4<T> m;
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            m[c][r] = src[c * 4 + r];
    return m;
}

template <typename T>
inline T LengthSquared(Vector2<T> v) {
    return v.x * v.x + v.y * v.y;
}
template <typename T>
inline T LengthSquared(Vector3<T> v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
template <typename T>
inline T LengthSquared(Vector4<T> v) {
    return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

template <typename T>
inline T Length(Vector2<T> v) {
    return std::sqrt(LengthSquared(v));
}
template <typename T>
inline T Length(Vector3<T> v) {
    return std::sqrt(LengthSquared(v));
}
template <typename T>
inline T Length(Vector4<T> v) {
    return std::sqrt(LengthSquared(v));
}

template <typename T>
inline auto Distance(T lhs, T rhs) {
    return Length(lhs - rhs);
}
template <typename T>
inline auto DistanceSquared(T lhs, T rhs) {
    return LengthSquared(lhs - rhs);
}

template <typename T>
inline Vector2<T> Normalize(Vector2<T> v) {
    return v / Length(v);
}
template <typename T>
inline Vector3<T> Normalize(Vector3<T> v) {
    return v / Length(v);
}
template <typename T>
inline Vector3<T> Normalize(Vector3<T> v, float &length) {
    length = Length(v);
    return v / length;
}
template <typename T>
inline Vector4<T> Normalize(Vector4<T> v) {
    return v / Length(v);
}

template <typename T>
inline T Dot(Vector2<T> lhs, Vector2<T> rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y;
}
template <typename T>
inline T Dot(Vector3<T> lhs, Vector3<T> rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}
template <typename T>
inline T Dot(Vector4<T> lhs, Vector4<T> rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}
template <typename T>
inline auto AbsDot(T lhs, T rhs) {
    return std::abs(Dot(lhs, rhs));
}
template <typename T>
inline auto ClampedDot(T lhs, T rhs) {
    auto d = Dot(lhs, rhs);
    return d > 0 ? d : (decltype(d))0;
}

template <typename T>
inline Vector3<T> Cross(Vector3<T> lhs, Vector3<T> rhs) {
    return {lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z,
            lhs.x * rhs.y - lhs.y * rhs.x};
}

template <typename T>
inline Vector2<T> Min(Vector2<T> lhs, Vector2<T> rhs) {
    return {std::min(lhs.x, rhs.x), std::min(lhs.y, rhs.y)};
}
template <typename T>
inline Vector3<T> Min(Vector3<T> lhs, Vector3<T> rhs) {
    return {std::min(lhs.x, rhs.x), std::min(lhs.y, rhs.y), std::min(lhs.z, rhs.z)};
}
template <typename T>
inline Vector4<T> Min(Vector4<T> lhs, Vector4<T> rhs) {
    return {std::min(lhs.x, rhs.x), std::min(lhs.y, rhs.y), std::min(lhs.z, rhs.z),
            std::min(lhs.w, rhs.w)};
}

template <typename T>
inline Vector2<T> Max(Vector2<T> lhs, Vector2<T> rhs) {
    return {std::max(lhs.x, rhs.x), std::max(lhs.y, rhs.y)};
}
template <typename T>
inline Vector3<T> Max(Vector3<T> lhs, Vector3<T> rhs) {
    return {std::max(lhs.x, rhs.x), std::max(lhs.y, rhs.y), std::max(lhs.z, rhs.z)};
}
template <typename T>
inline Vector4<T> Max(Vector4<T> lhs, Vector4<T> rhs) {
    return {std::max(lhs.x, rhs.x), std::max(lhs.y, rhs.y), std::max(lhs.z, rhs.z),
            std::max(lhs.w, rhs.w)};
}

template <typename T>
inline Vector2<T> Clamp(Vector2<T> val, Vector2<T> min, Vector2<T> max) {
    return {pine::Clamp(val.x, min.x, max.x), pine::Clamp(val.y, min.y, max.y)};
}
template <typename T>
inline Vector3<T> Clamp(Vector3<T> val, Vector3<T> min, Vector3<T> max) {
    return {pine::Clamp(val.x, min.x, max.x), pine::Clamp(val.y, min.y, max.y),
            pine::Clamp(val.z, min.z, max.z)};
}
template <typename T>
inline Vector4<T> Clamp(Vector4<T> val, Vector4<T> min, Vector4<T> max) {
    return {pine::Clamp(val.x, min.x, max.x), pine::Clamp(val.y, min.y, max.y),
            pine::Clamp(val.z, min.z, max.z), pine::Clamp(val.w, min.w, max.w)};
}

template <typename T>
inline Vector2<T> Fract(Vector2<T> val) {
    return {Fract(val.x), Fract(val.y)};
}
template <typename T>
inline Vector3<T> Fract(Vector3<T> val) {
    return {Fract(val.x), Fract(val.y), Fract(val.z)};
}
template <typename T>
inline Vector4<T> Fract(Vector4<T> val) {
    return {Fract(val.x), Fract(val.y), Fract(val.z), Fract(val.w)};
}

template <typename T>
inline Vector2<T> Floor(Vector2<T> val) {
    return {std::floor(val.x), std::floor(val.y)};
}
template <typename T>
inline Vector3<T> Floor(Vector3<T> val) {
    return {std::floor(val.x), std::floor(val.y), std::floor(val.z)};
}
template <typename T>
inline Vector4<T> Floor(Vector4<T> val) {
    return {std::floor(val.x), std::floor(val.y), std::floor(val.z), std::floor(val.w)};
}

template <typename T>
inline Vector2<T> Ceil(Vector2<T> val) {
    return {std::ceil(val.x), std::ceil(val.y)};
}
template <typename T>
inline Vector3<T> Ceil(Vector3<T> val) {
    return {std::ceil(val.x), std::ceil(val.y), std::ceil(val.z)};
}
template <typename T>
inline Vector4<T> Ceil(Vector4<T> val) {
    return {std::ceil(val.x), std::ceil(val.y), std::ceil(val.z), std::ceil(val.w)};
}

template <typename T>
inline Vector2<T> Sqrt(Vector2<T> val) {
    return {std::sqrt(val.x), std::sqrt(val.y)};
}
template <typename T>
inline Vector3<T> Sqrt(Vector3<T> val) {
    return {std::sqrt(val.x), std::sqrt(val.y), std::sqrt(val.z)};
}
template <typename T>
inline Vector4<T> Sqrt(Vector4<T> val) {
    return {std::sqrt(val.x), std::sqrt(val.y), std::sqrt(val.z), std::sqrt(val.w)};
}

template <typename T>
inline Vector2<T> Exp(Vector2<T> val) {
    return {std::exp(val.x), std::exp(val.y)};
}
template <typename T>
inline Vector3<T> Exp(Vector3<T> val) {
    return {std::exp(val.x), std::exp(val.y), std::exp(val.z)};
}
template <typename T>
inline Vector4<T> Exp(Vector4<T> val) {
    return {std::exp(val.x), std::exp(val.y), std::exp(val.z), std::exp(val.w)};
}

template <typename T>
inline Vector2<T> Pow(Vector2<T> val, T p) {
    return {std::pow(val.x, p), std::pow(val.y, p)};
}
template <typename T>
inline Vector3<T> Pow(Vector3<T> val, T p) {
    return {std::pow(val.x, p), std::pow(val.y, p), std::pow(val.z, p)};
}
template <typename T>
inline Vector4<T> Pow(Vector4<T> val, T p) {
    return {std::pow(val.x, p), std::pow(val.y, p), std::pow(val.z, p), std::pow(val.w, p)};
}

template <typename T>
inline Vector2<T> Abs(Vector2<T> val) {
    return {std::abs(val.x), std::abs(val.y)};
}
template <typename T>
inline Vector3<T> Abs(Vector3<T> val) {
    return {std::abs(val.x), std::abs(val.y), std::abs(val.z)};
}
template <typename T>
inline Vector4<T> Abs(Vector4<T> val) {
    return {std::abs(val.x), std::abs(val.y), std::abs(val.z), std::abs(val.w)};
}

template <typename T>
inline bool Inside(Vector2<T> p, Vector2<T> minInclude, Vector2<T> maxExclude) {
    return p.x >= minInclude.x && p.y >= minInclude.y && p.x < maxExclude.x && p.y < maxExclude.y;
}

template <typename T>
inline T TrilinearInterp(T c[2][2][2], Vector3<T> uvw) {
    T ret = 0;
    for (int x = 0; x < 2; x++)
        for (int y = 0; y < 2; y++)
            for (int z = 0; z < 2; z++) {
                ret += (x * uvw.x + (1 - x) * (1.0 - uvw.x)) *
                       (y * uvw.y + (1 - y) * (1.0 - uvw.y)) *
                       (z * uvw.z + (1 - z) * (1.0 - uvw.z)) * c[x][y][z];
            }
    return ret;
}

template <typename T>
inline T PerlinInterp(Vector3<T> c[2][2][2], Vector3<T> uvw) {
    T ret = 0;
    for (int x = 0; x < 2; x++)
        for (int y = 0; y < 2; y++)
            for (int z = 0; z < 2; z++) {
                Vector3<T> weight = uvw - Vector3<T>(x, y, z);
                ret += (x * uvw.x + (1 - x) * (1.0 - uvw.x)) *
                       (y * uvw.y + (1 - y) * (1.0 - uvw.y)) *
                       (z * uvw.z + (1 - z) * (1.0 - uvw.z)) * Dot(c[x][y][z], weight);
            }
    return ret;
}

template <typename T>
inline Matrix2<T> Transpose(const Matrix2<T> &m) {
    return {m.Row(0), m.Row(1)};
}
template <typename T>
inline Matrix3<T> Transpose(const Matrix3<T> &m) {
    return {m.Row(0), m.Row(1), m.Row(2)};
}
template <typename T>
inline Matrix4<T> Transpose(const Matrix4<T> &m) {
    return {m.Row(0), m.Row(1), m.Row(2), m.Row(3)};
}

// Floating-point vecmath

inline vec3 SafeRcp(vec3 v) {
    v.x = v.x == 0.0f ? 1e+20f : 1.0f / v.x;
    v.y = v.y == 0.0f ? 1e+20f : 1.0f / v.y;
    v.z = v.z == 0.0f ? 1e+20f : 1.0f / v.z;
    return v;
}
inline vec4 SafeRcp(vec4 v) {
    v.x = v.x == 0.0f ? 1e+20f : 1.0f / v.x;
    v.y = v.y == 0.0f ? 1e+20f : 1.0f / v.y;
    v.z = v.z == 0.0f ? 1e+20f : 1.0f / v.z;
    v.w = v.w == 0.0f ? 1e+20f : 1.0f / v.w;
    return v;
}

inline float Determinant(const mat3 &m) {
    return Dot(m.x, Cross(m.y, m.z));
}

inline mat2 Inverse(const mat2 &m) {
    float d = m[0][0] * m[1][1] - m[1][0] * m[0][1];
    // clang-format off
    return mat2(
     m[1][1], -m[1][0], 
    -m[0][1],  m[0][0]
    ) / d;
    // clang-format on
}

inline mat3 Inverse(const mat3 &m) {
    mat3 r;

    float determinant = m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2]) +
                        m[1][0] * (m[2][1] * m[0][2] - m[0][1] * m[2][2]) +
                        m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]);
    if (determinant == 0)
        return r;

    r[0][0] = m[1][1] * m[2][2] - m[2][1] * m[1][2];
    r[0][1] = m[2][1] * m[0][2] - m[0][1] * m[2][2];
    r[0][2] = m[0][1] * m[1][2] - m[1][1] * m[0][2];

    r[1][0] = m[1][2] * m[2][0] - m[2][2] * m[1][0];
    r[1][1] = m[2][2] * m[0][0] - m[0][2] * m[2][0];
    r[1][2] = m[0][2] * m[1][0] - m[1][2] * m[0][0];

    r[2][0] = m[1][0] * m[2][1] - m[2][0] * m[1][1];
    r[2][1] = m[2][0] * m[0][1] - m[0][0] * m[2][1];
    r[2][2] = m[0][0] * m[1][1] - m[1][0] * m[0][1];

    return r / determinant;
}

inline mat4 Inverse(const mat4 &m) {
    mat4 r;
    float determinant = 0;
    // clang-format off
    for (int i = 0; i < 4; i++)
        determinant += (m[(1 + i) % 4][0] * (m[(2 + i) % 4][1] *  m[(3 + i) % 4][2] - 
                                             m[(3 + i) % 4][1] *  m[(2 + i) % 4][2]) +
                        m[(2 + i) % 4][0] * (m[(3 + i) % 4][1] *  m[(1 + i) % 4][2] -  
                                             m[(1 + i) % 4][1] *  m[(3 + i) % 4][2]) +
                        m[(3 + i) % 4][0] * (m[(1 + i) % 4][1] *  m[(2 + i) % 4][2] -  
                                             m[(2 + i) % 4][1] *  m[(1 + i) % 4][2])) * 
                        m[i % 4][3] * (i % 2 ? -1 : 1);
    if (determinant == 0)
        return r;

    for (int v = 0; v < 4; v++)
        for (int i = 0; i < 4; i++)
            r[v][i] = (m[(1 + i) % 4][(1 + v) % 4] * (m[(2 + i) % 4][(2 + v) % 4] *  m[(3 + i) % 4][(3 + v) % 4] -  
                                                      m[(3 + i) % 4][(2 + v) % 4] *  m[(2 + i) % 4][(3 + v) % 4]) +
                       m[(2 + i) % 4][(1 + v) % 4] * (m[(3 + i) % 4][(2 + v) % 4] *  m[(1 + i) % 4][(3 + v) % 4] -  
                                                      m[(1 + i) % 4][(2 + v) % 4] *  m[(3 + i) % 4][(3 + v) % 4]) +
                       m[(3 + i) % 4][(1 + v) % 4] * (m[(1 + i) % 4][(2 + v) % 4] *  m[(2 + i) % 4][(3 + v) % 4] -  
                                                      m[(2 + i) % 4][(2 + v) % 4] *  m[(1 + i) % 4][(3 + v) % 4]))
                        * ((v + i) % 2 ? 1 : -1);

    // clang-format on
    return r / determinant;
}

inline mat4 Translate(float x, float y, float z) {
    // clang-format off
  return {1.0f, 0.0f, 0.0f, x, 
          0.0f, 1.0f, 0.0f, y,
          0.0f, 0.0f, 1.0f, z, 
          0.0f, 0.0f, 0.0f, 1.0f};
    // clang-format on
}

inline mat4 Translate(vec3 v) {
    // clang-format off
  return {1.0f, 0.0f, 0.0f, v.x, 
          0.0f, 1.0f, 0.0f, v.y,
          0.0f, 0.0f, 1.0f, v.z, 
          0.0f, 0.0f, 0.0f, 1.0f};
    // clang-format on
}

inline mat4 Scale(float x, float y, float z) {
    // clang-format off
  return {x, 0.0f, 0.0f, 0.0f,
          0.0f, y, 0.0f,0.0f,
          0.0f, 0.0f, z,0.0f,
          0.0f,0.0f,0.0f,1.0f};
    // clang-format on
}

inline mat4 Scale(vec3 v) {
    // clang-format off
  return {v.x, 0.0f, 0.0f,0.0f,
          0.0f, v.y, 0.0f,0.0f,
          0.0f, 0.0f, v.z,0.0f,
          0.0f,0.0f,0.0f,1.0f};
    // clang-format on
}

inline mat4 LookAt(vec3 from, vec3 at, vec3 up = vec3(0, 1, 0)) {
    vec3 z = Normalize(at - from);

    if (std::abs(Dot(z, up)) > 0.999f)
        z = Normalize(z + vec3(0.0f, 0.0f, 1e-5f));

    vec3 x = Normalize(Cross(up, z));
    vec3 y = Cross(z, x);
    return mat4((vec4)x, (vec4)y, (vec4)z, vec4(from, 1.0f));
}

inline mat3 CoordinateSystem(vec3 n) {
    vec3 t;
    if (std::abs(n.x) > std::abs(n.y))
        t = Normalize(Cross(n, vec3(0, 1, 0)));
    else
        t = Normalize(Cross(n, vec3(1, 0, 0)));
    vec3 b = Cross(n, t);
    return mat3(t, b, n);
}

inline vec3 SphericalToCartesian(float phi, float theta) {
    float sinTheta = std::sin(theta);
    return vec3(sinTheta * std::cos(phi), sinTheta * std::sin(phi), std::cos(theta));
}

inline float Phi2pi(float x, float y) {
    float phi = atan2f(y, x);
    return phi < 0.0f ? Pi * 2 + phi : phi;
}

inline vec2 CartesianToSpherical(vec3 d) {
    float phi = atan2f(d.y, d.x);
    if (phi < 0.0f)
        phi = Pi * 2 + phi;
    return vec2(phi, acosf(d.z));
}

inline uint32_t LeftShift32(uint32_t x) {
    if (x == (1 << 10))
        x--;
    x = (x | (x << 16)) & 0x30000ff;
    x = (x | (x << 8)) & 0x300f00f;
    x = (x | (x << 4)) & 0x30c30c3;
    x = (x | (x << 2)) & 0x9249249;
    return x;
}
inline uint64_t LeftShift64(uint64_t x) {
    uint64_t v = 0;
    for (int i = 0; i < 10; i++)
        v |= (1u << (i * 3)) & (x << (i * 2));
    return v;
}

inline uint32_t EncodeMorton32x3(vec3 v) {
    constexpr int kMortonBits = 10;
    constexpr int kMortonScale = 1 << kMortonBits;
    v *= kMortonScale;
    return (LeftShift32(v.z) << 2) | (LeftShift32(v.y) << 1) | LeftShift32(v.x);
}
inline uint64_t EncodeMorton64x3(vec3 v) {
    constexpr int kMortonBits = 20;
    constexpr int kMortonScale = 1 << kMortonBits;
    v *= kMortonScale;
    return (LeftShift64(v.z) << 2) | (LeftShift64(v.y) << 1) | LeftShift64(v.x);
}

}  // namespace pine

#endif  // PINE_CORE_VECMATH_H