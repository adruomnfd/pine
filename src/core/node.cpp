#include <core/node.h>
#include <util/fileio.h>
#include <util/parameters.h>

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
    std::unique_ptr<vec3u8[]> ptr(ReadLDRImage(filename, size));
    texels.assign(ptr.get(), ptr.get() + size.x * size.y);
}

vec3 nodes::Texture::EvalVec3(NodeEvalContext c) const {
    if (size == vec2i(0) || texels.size() == 0)
        return vec3(0.0f, 0.0f, 1.0f);
    vec2i co = size * pine::Fract(texcoord.EvalVec3(c));
    return pine::Pow(texels[co.x + co.y * size.x] / 255.0f, 2.2f);
}

Node* Node::Create(const Parameters& params) {
    std::string type = params.GetString("type");
    SWITCH(type) {
        CASE("Constant")
        return new nodes::Constant(params.GetFloat("float"), params.GetVec3("vec3"));
        CASE("Position")
        return new nodes::Position();
        CASE("Normal")
        return new nodes::Normal();
        CASE("TexCoord")
        return new nodes::TexCoord();
        CASE("Decompose")
        return new nodes::Decompose(Create(params["input"]), params.GetInt("dimension"));
        CASE("Composite")
        return new nodes::Composite(Create(params["inputX"]), Create(params["inputY"]),
                                    Create(params["inputZ"]));
        CASE("Add")
        return new nodes::Add(Create(params["input"]), Create(params["factor"]));
        CASE("Substract")
        return new nodes::Substract(Create(params["input"]), Create(params["factor"]));
        CASE("Multiply")
        return new nodes::Multiply(Create(params["input"]), Create(params["factor"]));
        CASE("Divide")
        return new nodes::Divide(Create(params["input"]), Create(params["factor"]));
        CASE("MultiplyAdd")
        return new nodes::MultiplyAdd(Create(params["input"]), Create(params["mulFactor"]),
                                      Create(params["addFactor"]));
        CASE("Length")
        return new nodes::Length(Create(params["input"]));
        CASE("Sqr")
        return new nodes::Sqr(Create(params["input"]));
        CASE("Sqrt")
        return new nodes::Sqrt(Create(params["input"]));
        CASE("Pow")
        return new nodes::Pow(Create(params["input"]), Create(params["exp"]));
        CASE("Sin")
        return new nodes::Sin(Create(params["input"]));
        CASE("Cos")
        return new nodes::Cos(Create(params["input"]));
        CASE("Tan")
        return new nodes::Tan(Create(params["input"]));
        CASE("Fract")
        return new nodes::Fract(Create(params["input"]));
        CASE("Checkerboard")
        return new nodes::Checkerboard(Create(params["input"]), Create(params["frequency"]));
        CASE("Noise")
        return new nodes::Noise(Create(params["input"]), Create(params["frequency"]),
                                Create(params["octaves"]));
        CASE("Noise3D")
        return new nodes::Noise3D(Create(params["input"]), Create(params["frequency"]),
                                  Create(params["octaves"]));
        CASE("Texture")
        return new nodes::Texture(Create(params["texcoord"]), params.GetString("filename"));
        CASE("Invert")
        return new nodes::Invert(Create(params["input"]));
        DEFAULT {
            LOG_WARNING("[NodeInput][Create]Unknown type \"&\"", type);
            return new nodes::Constant(params.GetFloat("float", 0.5f),
                                       params.GetVec3("vec3", vec3(0.5f)));
        }
    }
}

}  // namespace pine