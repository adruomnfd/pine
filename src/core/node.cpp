#include <core/node.h>
#include <util/fileio.h>
#include <util/parameters.h>

namespace pine {

float NodeInput::EvalFloat(const NodeEvalCtx& c) const {
    if (link)
        return link->EvalFloat(c);
    else
        return defaultFloat;
}
vec3 NodeInput::EvalVec3(const NodeEvalCtx& c) const {
    if (link)
        return link->EvalVec3(c);
    else
        return defaultVec3;
}

nodes::Texture::Texture(NodeInput texcoord, pstd::string_view filename) : texcoord(texcoord) {
    LOG("[Texture]Loading \"&\"", filename);
    pstd::unique_ptr<vec3u8[]> ptr(ReadLDRImage(filename, size));
    texels.assign(ptr.get(), ptr.get() + Area(size));
}

vec3 nodes::Texture::EvalVec3(const NodeEvalCtx& c) const {
    if (size == vec2i(0) || texels.size() == 0)
        return vec3(0.0f, 0.0f, 1.0f);
    vec2i co = size * pine::Fract(texcoord.EvalVec3(c));
    return pine::Pow(texels[co.x + co.y * size.x] / 255.0f, 2.2f);
}

Node* CreateNode(const Parameters& params) {
    pstd::string type = params.GetString("type");
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
        return new nodes::Decompose(CreateNode(params["input"]), params.GetInt("dimension"));
        CASE("Composite")
        return new nodes::Composite(CreateNode(params["inputX"]), CreateNode(params["inputY"]),
                                    CreateNode(params["inputZ"]));
        CASE("Add")
        return new nodes::Add(CreateNode(params["input"]), CreateNode(params["factor"]));
        CASE("Substract")
        return new nodes::Substract(CreateNode(params["input"]), CreateNode(params["factor"]));
        CASE("Multiply")
        return new nodes::Multiply(CreateNode(params["input"]), CreateNode(params["factor"]));
        CASE("Divide")
        return new nodes::Divide(CreateNode(params["input"]), CreateNode(params["factor"]));
        CASE("MultiplyAdd")
        return new nodes::MultiplyAdd(CreateNode(params["input"]), CreateNode(params["mulFactor"]),
                                      CreateNode(params["addFactor"]));
        CASE("Length")
        return new nodes::Length(CreateNode(params["input"]));
        CASE("Sqr")
        return new nodes::Sqr(CreateNode(params["input"]));
        CASE("Sqrt")
        return new nodes::Sqrt(CreateNode(params["input"]));
        CASE("Pow")
        return new nodes::Pow(CreateNode(params["input"]), CreateNode(params["exp"]));
        CASE("Sin")
        return new nodes::Sin(CreateNode(params["input"]));
        CASE("Cos")
        return new nodes::Cos(CreateNode(params["input"]));
        CASE("Tan")
        return new nodes::Tan(CreateNode(params["input"]));
        CASE("Fract")
        return new nodes::Fract(CreateNode(params["input"]));
        CASE("Checkerboard")
        return new nodes::Checkerboard(CreateNode(params["input"]),
                                       CreateNode(params["frequency"]));
        CASE("Noise")
        return new nodes::Noise(CreateNode(params["input"]), CreateNode(params["frequency"]),
                                CreateNode(params["octaves"]));
        CASE("Noise3D")
        return new nodes::Noise3D(CreateNode(params["input"]), CreateNode(params["frequency"]),
                                  CreateNode(params["octaves"]));
        CASE("Texture")
        return new nodes::Texture(CreateNode(params["texcoord"]), params.GetString("filename"));
        CASE("Invert")
        return new nodes::Invert(CreateNode(params["input"]));
        CASE("" || params.GetString("@") != "")
        return new nodes::Constant(params.GetFloat("float", params.GetFloat("@")),
                                   params.GetVec3("vec3", params.GetVec3("@")));
        DEFAULT {
            LOG_WARNING("[NodeInput][Create]Unknown type \"&\"", type);
            return new nodes::Constant(params.GetFloat("float", 0.5f),
                                       params.GetVec3("vec3", vec3(0.5f)));
        }
    }
}

}  // namespace pine