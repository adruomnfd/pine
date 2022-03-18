#include <core/vecmath.h>
#include <core/spectrum.h>
#include <util/log.h>

using namespace pine;

#define ExpectEQ(a, b) CHECK_LT(Length(a - b), 0.001f)

int main(int, char**) {
    {
        vec4 p = vec4(1, 2, 3, 1);
        mat4 m = LookAt(vec3(2, 7, 5), vec3(8, 2, 1));
        mat4 mi = Inverse(m);
        ExpectEQ(mi * m * p, p)
    }
    {
        mat2 m2 = {2, 3, 4, 5};
        ExpectEQ(Inverse(m2) * m2 * vec2(7, 8), vec2i(7, 8));
    }
    SampledSpectrum::Init();
    Spectrum color(1.0f);
    color *= 4.0f;
    print(color.ToRGB());
}