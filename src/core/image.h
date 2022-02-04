#ifndef PINE_CORE_IMAGE_H
#define PINE_CORE_IMAGE_H

#include <core/vecmath.h>
#include <util/log.h>

#include <vector>

namespace pine {

struct Image {
    Image() = default;
    Image(vec2i size) : size_(size) {
        colors.resize((size_t)size_.x * size_.y);
    };

    vec4& operator[](vec2i p) {
        DCHECK_GE(p.x, 0);
        DCHECK_GE(p.y, 0);
        DCHECK_LT(p.x, size_.x);
        DCHECK_LT(p.y, size_.y);
        return colors[p.x + p.y * size_.x];
    }
    const vec4& operator[](vec2i p) const {
        DCHECK_GE(p.x, 0);
        DCHECK_GE(p.y, 0);
        DCHECK_LT(p.x, size_.x);
        DCHECK_LT(p.y, size_.y);
        return colors[p.x + p.y * size_.x];
    }

    const vec4* Data() const {
        return colors.data();
    }
    vec2i Size() const {
        return size_;
    }
    size_t SizeInBytes() const {
        return sizeof(colors[0]) * size_.x * size_.y;
    }

  private:
    vec2i size_;
    std::vector<vec4> colors;
};

}  // namespace pine

#endif  // PINE_CORE_IMAGE_H