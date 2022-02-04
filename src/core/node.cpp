#include <core/node.h>
#include <util/fileio.h>
#include <util/parameters.h>

#define STB_IMAGE_IMPLEMENTATION
#include <ext/stb_image.h>

namespace pine {

float NodeInput::EvalFloat(NodeEvalContext c) const {
    if (link)
        return link->EvalFloat(c);
    else
        return defaultFloat;
}
vec3 NodeInput::EvalVec3(NodeEvalContext c) const {
    if (link)
        return link->EvalVec3(c);
    else
        return defaultVec3;
}

nodes::Texture::Texture(NodeInput texcoord, std::string filename) : texcoord(texcoord) {
    filename = sceneDirectory + filename;
    LOG_VERBOSE("[Texture]Loading \"&\"", filename);
    int nchannel;
    stbi_uc* data = stbi_load(filename.c_str(), &size.x, &size.y, &nchannel, 3);
    if (data == nullptr)
        LOG_WARNING("[Texture][Ctor]Fail to load \"&\"", filename);
    texels.resize((size_t)size.x * size.y);
    memcpy(texels.data(), data, (size_t)size.x * size.y * 3);
}

vec3 nodes::Texture::EvalVec3(NodeEvalContext c) const {
    if (size == vec2i(0) || texels.size() == 0)
        return vec3(0.0f, 0.0f, 1.0f);
    vec2i co = size * pine::Fract(texcoord.EvalVec3(c));
    return pine::Pow(texels[co.x + co.y * size.x] / 255.0f, 2.2f);
}

Node* Node::Create(const Parameters& params) {
    std::string type = params.GetString("type");
    if (type == "Constant") {
        return new nodes::Constant(params.GetFloat("float"), params.GetVec3("vec3"));
    } else if (type == "Position") {
        return new nodes::Position();
    } else if (type == "Normal") {
        return new nodes::Normal();
    } else if (type == "TexCoord") {
        return new nodes::TexCoord();
    } else if (type == "Decompose") {
        return new nodes::Decompose(Create(params["input"]), params.GetInt("dimension"));
    } else if (type == "Composite") {
        return new nodes::Composite(Create(params["inputX"]), Create(params["inputY"]),
                                    Create(params["inputZ"]));
    } else if (type == "Add") {
        return new nodes::Add(Create(params["input"]), Create(params["factor"]));
    } else if (type == "Substract") {
        return new nodes::Substract(Create(params["input"]), Create(params["factor"]));
    } else if (type == "Multiply") {
        return new nodes::Multiply(Create(params["input"]), Create(params["factor"]));
    } else if (type == "Divide") {
        return new nodes::Divide(Create(params["input"]), Create(params["factor"]));
    } else if (type == "MultiplyAdd") {
        return new nodes::MultiplyAdd(Create(params["input"]), Create(params["mulFactor"]),
                                      Create(params["addFactor"]));
    } else if (type == "Length") {
        return new nodes::Length(Create(params["input"]));
    } else if (type == "Sqr") {
        return new nodes::Sqr(Create(params["input"]));
    } else if (type == "Sqrt") {
        return new nodes::Sqrt(Create(params["input"]));
    } else if (type == "Pow") {
        return new nodes::Pow(Create(params["input"]), Create(params["power"]));
    } else if (type == "Sin") {
        return new nodes::Sin(Create(params["input"]));
    } else if (type == "Cos") {
        return new nodes::Cos(Create(params["input"]));
    } else if (type == "Tan") {
        return new nodes::Tan(Create(params["input"]));
    } else if (type == "Fract") {
        return new nodes::Fract(Create(params["input"]));
    } else if (type == "Checkboard") {
        return new nodes::Checkboard(Create(params["input"]), Create(params["frequency"]));
    } else if (type == "Noise") {
        return new nodes::Noise(Create(params["input"]), Create(params["frequency"]),
                                Create(params["octaves"]));
    } else if (type == "Noise3D") {
        return new nodes::Noise3D(Create(params["input"]), Create(params["frequency"]),
                                  Create(params["octaves"]));
    } else if (type == "Texture") {
        return new nodes::Texture(Create(params["texcoord"]), params.GetString("filename"));
    } else if (type == "Invert") {
        return new nodes::Invert(Create(params["input"]));
    } else {
        LOG_WARNING("[NodeInput][Create]Unknown type \"&\"", type);
        return new nodes::Constant(params.GetFloat("float", 0.5f),
                                   params.GetVec3("vec3", vec3(0.5f)));
    }
}

}  // namespace pine