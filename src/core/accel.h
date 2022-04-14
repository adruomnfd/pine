#ifndef PINE_CORE_ACCEL_H
#define PINE_CORE_ACCEL_H

#include <core/geometry.h>

#include <memory>
#include <vector>

namespace pine {

class Accel {
  public:
    virtual ~Accel() = default;
    virtual void Initialize(const Scene* scene) = 0;
    virtual bool Hit(Ray ray) const = 0;
    virtual bool Intersect(Ray& ray, Interaction& it) const = 0;
};

Accel* CreateAccel(const Parameters& params);

}  // namespace pine

#endif  // PINE_CORE_ACCEL_H