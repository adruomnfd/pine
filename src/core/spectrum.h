#ifndef PINE_CORE_SPECTRUM_H
#define PINE_CORE_SPECTRUM_H

#include <core/vecmath.h>

namespace pine {

enum class SpectrumType { Reflectance, Illuminant };
static const int sampledLambdaStart = 400;
static const int sampledLambdaEnd = 700;
static const int nSpectralSamples = 60;
static const int nCIESamples = 471;
extern const float CIE_X[nCIESamples];
extern const float CIE_Y[nCIESamples];
extern const float CIE_Z[nCIESamples];
extern const float CIE_lambda[nCIESamples];
static const float CIE_Y_integral = 106.856895f;
static const int nRGB2SpectSamples = 32;
extern const float RGB2SpectLambda[nRGB2SpectSamples];
extern const float RGBRefl2SpectWhite[nRGB2SpectSamples];
extern const float RGBRefl2SpectCyan[nRGB2SpectSamples];
extern const float RGBRefl2SpectMagenta[nRGB2SpectSamples];
extern const float RGBRefl2SpectYellow[nRGB2SpectSamples];
extern const float RGBRefl2SpectRed[nRGB2SpectSamples];
extern const float RGBRefl2SpectGreen[nRGB2SpectSamples];
extern const float RGBRefl2SpectBlue[nRGB2SpectSamples];
extern const float RGBIllum2SpectWhite[nRGB2SpectSamples];
extern const float RGBIllum2SpectCyan[nRGB2SpectSamples];
extern const float RGBIllum2SpectMagenta[nRGB2SpectSamples];
extern const float RGBIllum2SpectYellow[nRGB2SpectSamples];
extern const float RGBIllum2SpectRed[nRGB2SpectSamples];
extern const float RGBIllum2SpectGreen[nRGB2SpectSamples];
extern const float RGBIllum2SpectBlue[nRGB2SpectSamples];

inline void XYZToRGB(const float xyz[3], float rgb[3]) {
    rgb[0] = 3.240479f * xyz[0] - 1.537150f * xyz[1] - 0.498535f * xyz[2];
    rgb[1] = -0.969256f * xyz[0] + 1.875991f * xyz[1] + 0.041556f * xyz[2];
    rgb[2] = 0.055648f * xyz[0] - 0.204043f * xyz[1] + 1.057311f * xyz[2];
}
inline void RGBToXYZ(const float rgb[3], float xyz[3]) {
    xyz[0] = 0.412453f * rgb[0] + 0.357580f * rgb[1] + 0.180423f * rgb[2];
    xyz[1] = 0.212671f * rgb[0] + 0.715160f * rgb[1] + 0.072169f * rgb[2];
    xyz[2] = 0.019334f * rgb[0] + 0.119193f * rgb[1] + 0.950227f * rgb[2];
}

class RGBSpectrum;

template <int nSpectrumSamples>
class CoefficientSpectrum {
  public:
    CoefficientSpectrum(float v = 0.0f) {
        for (int i = 0; i < nSpectrumSamples; i++)
            c[i] = v;
    }

    CoefficientSpectrum& operator+=(const CoefficientSpectrum& s2) {
        for (int i = 0; i < nSpectrumSamples; i++)
            c[i] += s2.c[i];
        return *this;
    }
    CoefficientSpectrum& operator-=(const CoefficientSpectrum& s2) {
        for (int i = 0; i < nSpectrumSamples; i++)
            c[i] -= s2.c[i];
        return *this;
    }
    CoefficientSpectrum& operator*=(const CoefficientSpectrum& s2) {
        for (int i = 0; i < nSpectrumSamples; i++)
            c[i] *= s2.c[i];
        return *this;
    }
    CoefficientSpectrum& operator/=(const CoefficientSpectrum& s2) {
        for (int i = 0; i < nSpectrumSamples; i++)
            c[i] /= s2.c[i];
        return *this;
    }

    friend CoefficientSpectrum operator+(CoefficientSpectrum s1, const CoefficientSpectrum& s2) {
        return s1 += s2;
    }
    friend CoefficientSpectrum operator-(CoefficientSpectrum s1, const CoefficientSpectrum& s2) {
        return s1 -= s2;
    }
    friend CoefficientSpectrum operator*(CoefficientSpectrum s1, const CoefficientSpectrum& s2) {
        return s1 *= s2;
    }
    friend CoefficientSpectrum operator/(CoefficientSpectrum s1, const CoefficientSpectrum& s2) {
        return s1 /= s2;
    }

    friend CoefficientSpectrum Sqrt(CoefficientSpectrum s) {
        for (int i = 0; i < nSpectrumSamples; i++)
            s.c[i] = std::sqrt(s.c[i]);
        return s;
    }

    friend CoefficientSpectrum Pow(CoefficientSpectrum s, float exp) {
        for (int i = 0; i < nSpectrumSamples; i++)
            s.c[i] = std::pow(s.c[i], exp);
        return s;
    }

    friend CoefficientSpectrum Exp(CoefficientSpectrum s) {
        for (int i = 0; i < nSpectrumSamples; i++)
            s.c[i] = std::exp(s.c[i]);
        return s;
    }

    friend CoefficientSpectrum Clamp(CoefficientSpectrum s, float low = 0, float high = Infinity) {
        for (int i = 0; i < nSpectrumSamples; i++)
            s.c[i] = pine::Clamp(s.c[i], low, high);
        return s;
    }

    friend CoefficientSpectrum Lerp(float t, const CoefficientSpectrum& s1,
                                    const CoefficientSpectrum& s2) {
        return (1 - t) * s1 + t * s2;
    }

    CoefficientSpectrum operator-() const {
        CoefficientSpectrum s2 = *this;
        for (int i = 0; i < nSpectrumSamples; i++)
            s2.c[i] = -c[i];
        return s2;
    }

    float& operator[](int i) {
        return c[i];
    }
    float operator[](int i) const {
        return c[i];
    }

    bool IsBlack() const {
        for (int i = 0; i < nSpectrumSamples; i++)
            if (c[i] != 0.0f)
                return false;
        return true;
    }

    bool HasNaNs() const {
        for (int i = 0; i < nSpectrumSamples; i++)
            if (std::isnan(c[i]))
                return true;
        return false;
    }

    float c[nSpectrumSamples];
};

class SampledSpectrum : public CoefficientSpectrum<nSpectralSamples> {
  public:
    using CoefficientSpectrum::CoefficientSpectrum;
    SampledSpectrum(const CoefficientSpectrum& s) : CoefficientSpectrum(s) {
    }
    SampledSpectrum(vec3 rgb, SpectrumType type = SpectrumType::Reflectance) {
        *this = FromRGB(&rgb[0], type);
    }

    static void Initialize();

    static SampledSpectrum FromSampled(const float* lambda, const float* v, int n);
    static SampledSpectrum FromRGB(const float rgb[3],
                                   SpectrumType type = SpectrumType::Reflectance);
    static SampledSpectrum FromXYZ(const float xyz[3],
                                   SpectrumType type = SpectrumType::Reflectance);

    RGBSpectrum ToRGBSpectrum() const;
    void ToXYZ(float xyz[3]) const {
        xyz[0] = xyz[1] = xyz[2] = 0;
        for (int i = 0; i < nSpectralSamples; i++) {
            xyz[0] += X.c[i] * c[i];
            xyz[1] += Y.c[i] * c[i];
            xyz[2] += Z.c[i] * c[i];
        }
        float scale =
            float(sampledLambdaEnd - sampledLambdaStart) / (CIE_Y_integral * nSpectralSamples);
        xyz[0] *= scale;
        xyz[1] *= scale;
        xyz[2] *= scale;
    }
    void ToRGB(float rgb[3]) const {
        float xyz[3];
        ToXYZ(xyz);
        XYZToRGB(xyz, rgb);
    }
    vec3 ToRGB() const {
        vec3 xyz;
        ToRGB(&xyz[0]);
        return xyz;
    }
    float y() const {
        float yy = 0.0f;
        for (int i = 0; i < nSpectralSamples; i++)
            yy += Y.c[i] * c[i];
        return yy * float(sampledLambdaEnd - sampledLambdaStart) /
               (CIE_Y_integral * nSpectralSamples);
    }

  private:
    static SampledSpectrum X, Y, Z;
    static SampledSpectrum rgbRefl2SpectWhite, rgbRefl2SpectCyan;
    static SampledSpectrum rgbRefl2SpectMagenta, rgbRefl2SpectYellow;
    static SampledSpectrum rgbRefl2SpectRed, rgbRefl2SpectGreen;
    static SampledSpectrum rgbRefl2SpectBlue;
    static SampledSpectrum rgbIllum2SpectWhite, rgbIllum2SpectCyan;
    static SampledSpectrum rgbIllum2SpectMagenta, rgbIllum2SpectYellow;
    static SampledSpectrum rgbIllum2SpectRed, rgbIllum2SpectGreen;
    static SampledSpectrum rgbIllum2SpectBlue;
};

class RGBSpectrum : public CoefficientSpectrum<3> {
  public:
    using CoefficientSpectrum::CoefficientSpectrum;
    RGBSpectrum(const CoefficientSpectrum& s) : CoefficientSpectrum(s) {
    }
    RGBSpectrum(vec3 rgb, SpectrumType type = SpectrumType::Reflectance) {
        *this = FromRGB(&rgb[0], type);
    }

    static RGBSpectrum FromSampled(const float* lambda, const float* v, int n);

    static RGBSpectrum FromRGB(const float rgb[3], SpectrumType = SpectrumType::Reflectance) {
        RGBSpectrum s;
        s.c[0] = rgb[0];
        s.c[1] = rgb[1];
        s.c[2] = rgb[2];
        return s;
    }
    static RGBSpectrum FromXYZ(const float xyz[3], SpectrumType = SpectrumType::Reflectance) {
        RGBSpectrum s;
        XYZToRGB(xyz, s.c);
        return s;
    }

    const RGBSpectrum& ToRGBSpectrum() const {
        return *this;
    }
    void ToRGB(float rgb[3]) const {
        rgb[0] = c[0];
        rgb[1] = c[1];
        rgb[2] = c[2];
    }
    void ToXYZ(float xyz[3]) const {
        RGBToXYZ(c, xyz);
    }
    vec3 ToRGB() const {
        vec3 xyz;
        ToRGB(&xyz[0]);
        return xyz;
    }
    float y() const {
        const float YWeight[3] = {0.212671f, 0.715160f, 0.072169f};
        return YWeight[0] * c[0] + YWeight[1] * c[1] + YWeight[2] * c[2];
    }
};

typedef RGBSpectrum Spectrum;

}  // namespace pine

#endif  // PINE_CORE_SPECTRUM_H