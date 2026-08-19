// Cynth's Objects — a pack of custom objects for Geometry Dash (Geode).
//
// Built on top of Object Collab (smjs.object-collab), which provides the
// custom-object framework and cross-mod compatibility layer.
//
// Currently the only object is a "VHS Shader": a decorative object whose
// sprite is rendered through a custom GLSL fragment shader that gives it a
// retro VHS look (chromatic aberration, scanlines, film grain, vignette).

#include <Geode/Geode.hpp>
// `CCGLProgram` normally comes in transitively via <Geode/Geode.hpp>. If your
// compiler can't find it, also include the cocos shader header explicitly:
//   #include <Geode/cocos/include/shaders/CCGLProgram.h>
#include <smjs.object-collab/include/object_collab.hpp>

using namespace geode::prelude;
using namespace object_collab::prelude;

// ---------------------------------------------------------------------------
// VHS shader
// ---------------------------------------------------------------------------

// Standard cocos2d-x vertex shader (position + color + texture coordinates).
static constexpr const char* VHS_VERTEX_SHADER = R"(
attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color;
uniform mat4 CC_MVPMatrix;

varying vec2 v_texCoord;
varying vec4 v_fragmentColor;

void main() {
    gl_Position = CC_MVPMatrix * a_position;
    v_texCoord = a_texCoord;
    v_fragmentColor = a_color;
}
)";

// VHS-style fragment shader. `CC_Texture0` is the cocos2d built-in sampler.
static constexpr const char* VHS_FRAGMENT_SHADER = R"(
#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;
varying vec4 v_fragmentColor;

uniform sampler2D CC_Texture0;

void main() {
    vec2 uv = v_texCoord;

    // Chromatic aberration — sample each channel with a slight radial offset.
    vec2 dir = uv - 0.5;
    float aberration = 0.004;
    float r = texture2D(CC_Texture0, uv + dir * aberration).r;
    float g = texture2D(CC_Texture0, uv).g;
    float b = texture2D(CC_Texture0, uv - dir * aberration).b;
    vec3 color = vec3(r, g, b);

    // Scanlines.
    color *= 0.85 + 0.15 * sin(uv.y * 240.0);

    // Film grain / noise.
    float n = fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
    color += (n - 0.5) * 0.06;

    // Vignette.
    color *= 1.0 - dot(uv - 0.5, uv - 0.5) * 0.7;

    gl_FragColor = v_fragmentColor * vec4(color, 1.0);
}
)";

// Builds (once) and caches the shared VHS shader program.
static cocos2d::CCGLProgram* vhsProgram() {
    static cocos2d::CCGLProgram* program = [] {
        auto* p = new cocos2d::CCGLProgram();
        p->initWithVertexShaderByteArray(VHS_VERTEX_SHADER, VHS_FRAGMENT_SHADER);
        p->addAttribute(cocos2d::kCCAttributeNamePosition, cocos2d::kCCVertexAttrib_Position);
        p->addAttribute(cocos2d::kCCAttributeNameColor, cocos2d::kCCVertexAttrib_Color);
        p->addAttribute(cocos2d::kCCAttributeNameTexCoord, cocos2d::kCCVertexAttrib_TexCoords);
        p->link();
        p->updateUniforms();
        return p;
    }();
    return program;
}

// ---------------------------------------------------------------------------
// The VHS Shader object
// ---------------------------------------------------------------------------

class $object(VHSShaderObject, GameObject) {
public:
    static VHSShaderObject* create(ObjectInfo* info) {
        return new VHSShaderObject(info);
    }

    VHSShaderObject(ObjectInfo* info)
        : CustomObject(info, ObjectTraits::builder()
            .gameObjectType(GameObjectType::Decoration)
            .build()) {}

    // Runs after the object has fully generated (both editor and play).
    // Recommended place to tweak default GameObject properties.
    void postInit() override {
        auto* program = vhsProgram();

        // The object itself is the primary sprite.
        this->setShaderProgram(program);

        // Also apply to the colored detail sprite if one exists.
        if (auto* colorSprite = this->getColorSprite()) {
            colorSprite->setShaderProgram(program);
        }
    }

    std::vector<std::string> getObjectDetails() override {
        return DetailsBuilder::builder()
            .field("Effect", "VHS")
            .build();
    }
};

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------

$on_mod(Loaded) {
    ObjectAPI::registerObject(ObjectInfo::builder()
        .id("vhs-shader"_spr)
        .sprite("vhs.png"_spr)
        .editorTab(EditorTab::Decorations)
        .editorButtonColor(EditorButtonColor::Pink)
        .construction(ComplexObject::builder()
            .factory(VHSShaderObject::create)
            .build())
        .build());
}
