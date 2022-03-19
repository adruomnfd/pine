#include <core/film.h>
#include <core/color.h>
#include <util/parameters.h>
#include <util/fileio.h>

namespace pine {

Film Film::Create(const Parameters& params) {
    return Film(params.GetVec2i("size", vec2i(720, 480)), Filter::Create(params["filter"]),
                params.GetString("outputFileName", "result.png"));
}

Film::Film(vec2i size, Filter filter, std::string outputFileName)
    : size(size), filter(filter), outputFileName(outputFileName) {
    int offset = 0;
    for (int y = 0; y < filterTableWidth; y++)
        for (int x = 0; x < filterTableWidth; x++) {
            vec2 p = {(x + 0.5f) / filterTableWidth * filter.Radius().x,
                      (y + 0.5f) / filterTableWidth * filter.Radius().y};
            filterTable[offset++] = filter.Evaluate(p);
        }
    pixels = std::shared_ptr<Pixel[]>(new Pixel[size.x * size.y]);
    rgba = std::shared_ptr<vec4[]>(new vec4[size.x * size.y]);
}

void Film::WriteToDisk(std::string filename) const {
    std::unique_ptr<vec4u8[]> rgba8 = std::unique_ptr<vec4u8[]>(new vec4u8[size.x * size.y]);
    for (int i = 0; i < size.x * size.y; i++)
        rgba8[i] = rgba[i] * 255;
    SaveImage(filename, size, 4, (float*)&rgba[0]);
}

void Film::CopyToRGBArray() {
    for (int i = 0; i < size.x * size.y; i++) {
        float invWeight = 1.0f / (float)pixels[i].weight;
        for (int c = 0; c < 4; c++)
            rgba[i][c] = float(pixels[i].rgba[c]) * invWeight;
    }
}
void Film::ApplyToneMapping() {
    for (int i = 0; i < size.x * size.y; i++)
        rgba[i] = vec4(Uncharted2Flimic(rgba[i]), rgba[i].w);
}
void Film::ApplyGammaCorrection() {
    for (int i = 0; i < size.x * size.y; i++)
        rgba[i] = vec4(Pow((vec3)rgba[i], 1.0f / 2.2f), rgba[i].w);
}

}  // namespace pine