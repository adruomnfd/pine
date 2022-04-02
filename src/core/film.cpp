#include <core/film.h>
#include <core/color.h>
#include <util/parameters.h>
#include <util/fileio.h>

namespace pine {

Film Film::Create(const Parameters& params) {
    return Film(params.GetVec2i("size", vec2i(720, 480)), Filter::Create(params["filter"]),
                params.GetString("outputFileName", "result.png"),
                params.GetBool("applyToneMapping", true), params.GetBool("reportAverageColor", false));
}

Film::Film(vec2i size, Filter filter, std::string outputFileName, bool applyToneMapping,
           bool reportAverageColor)
    : size(size),
      filter(filter),
      outputFileName(outputFileName),
      applyToneMapping(applyToneMapping),
      reportAverageColor(reportAverageColor) {
    int offset = 0;
    for (int y = 0; y < filterTableWidth; y++)
        for (int x = 0; x < filterTableWidth; x++) {
            vec2 p = {(x + 0.5f) / filterTableWidth * filter.Radius().x,
                      (y + 0.5f) / filterTableWidth * filter.Radius().y};
            filterTable[offset++] = filter.Evaluate(p);
        }
    pixels = std::shared_ptr<Pixel[]>(new Pixel[Area(size)]);
    rgba = std::shared_ptr<vec4[]>(new vec4[Area(size)]);
}

void Film::Clear() {
    for (int i = 0; i < Area(size); i++) {
        pixels[i].rgb[0] = 0.0f;
        pixels[i].rgb[1] = 0.0f;
        pixels[i].rgb[2] = 0.0f;
        pixels[i].splatXYZ[0] = {};
        pixels[i].splatXYZ[1] = {};
        pixels[i].splatXYZ[2] = {};
        pixels[i].weight = 0.0f;
        rgba[i] = {};
    }
}
void Film::Finalize(float splatMultiplier) {
    CopyToRGBArray(splatMultiplier);
    
    if (reportAverageColor) {
        vec4 avg;
        for (int i = 0; i < Area(size); i++)
            avg += rgba[i];
        avg /= Area(size);
        LOG("[Film]Average RGB color: &", avg);
    }

    if (applyToneMapping)
        ApplyToneMapping();

    ApplyGammaCorrection();

    if (frameId == 0)
        WriteToDisk(outputFileName);
    else
        WriteToDisk(AppendFileName(outputFileName, ToString("_frame_", frameId)));
    frameId++;
}
void Film::WriteToDisk(std::string filename) const {
    std::unique_ptr<vec4u8[]> rgba8 = std::unique_ptr<vec4u8[]>(new vec4u8[Area(size)]);
    for (int i = 0; i < Area(size); i++)
        rgba8[i] = rgba[i] * 255;
    SaveImage(filename, size, 4, (float*)&rgba[0]);
}

void Film::CopyToRGBArray(float splatMultiplier) {
    for (int i = 0; i < Area(size); i++) {
        auto& pixel = pixels[i];
        if (pixel.weight != 0.0f)
            for (int c = 0; c < 3; c++)
                rgba[i][c] = float(pixel.rgb[c]) / (float)pixel.weight;
        else
            rgba[i] = {};

        float splatXYZ[3] = {pixel.splatXYZ[0], pixel.splatXYZ[1], pixel.splatXYZ[2]};
        float splatRGB[3];
        XYZToRGB(splatXYZ, splatRGB);
        rgba[i][0] += splatRGB[0] * splatMultiplier;
        rgba[i][1] += splatRGB[1] * splatMultiplier;
        rgba[i][2] += splatRGB[2] * splatMultiplier;

        rgba[i][3] = 1.0f;
    }
}
void Film::ApplyToneMapping() {
    for (int i = 0; i < Area(size); i++)
        rgba[i] = vec4(Uncharted2Flimic(rgba[i]), rgba[i].w);
}
void Film::ApplyGammaCorrection() {
    for (int i = 0; i < Area(size); i++)
        rgba[i] = vec4(Pow((vec3)rgba[i], 1.0f / 2.2f), rgba[i].w);
}

}  // namespace pine