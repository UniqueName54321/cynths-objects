// Cynth's Objects — a pack of custom objects for Geometry Dash (Geode).
//
// Built on top of Object Collab (smjs.object-collab), which provides the
// custom-object framework and cross-mod compatibility layer.
//
// Currently the only object is a "VHS Overlay": an invisible-in-gameplay
// marker object that spawns a full-screen overlay rendered through a custom
// GLSL shader (scanlines, film grain, vignette, color fringing, tracking
// lines). Its translucency is adjustable from the object's "Edit Special"
// screen, exactly like a vanilla shader object.

#include <algorithm>

#include <Geode/Geode.hpp>
// `CCGLProgram` normally comes in transitively via <Geode/Geode.hpp>. If your
// compiler can't find it, also include the cocos shader header explicitly:
//   #include <Geode/cocos/include/shaders/CCGLProgram.h>
#include <Geode/modify/PlayLayer.hpp>
#include <smjs.object-collab/include/object_collab.hpp>

using namespace geode::prelude;
using namespace object_collab::prelude;

// ---------------------------------------------------------------------------
// VHS overlay shader
// ---------------------------------------------------------------------------

// Standard cocos2d-x vertex shader (position + color + texture coordinates).
static constexpr const char* VHS_OVERLAY_VERTEX = R"(
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

// Full-screen VHS-style fragment shader. It composites over the gameplay
// scene via normal alpha blending, so we only synthesize the *effect* here
// (no texture sampling — the underlying sprite is just a solid white quad).
//
// To animate the tracking lines, add `uniform float time;` and drive it from a
// per-frame update, then use it in the `track`/`roll` terms below.
static constexpr const char* VHS_OVERLAY_FRAGMENT = R"(
#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;
varying vec4 v_fragmentColor;

uniform sampler2D CC_Texture0; // declared for cocos2d compatibility (unused)

void main() {
    vec2 uv = v_texCoord;
    vec3 color = v_fragmentColor.rgb;

    // Horizontal scanlines.
    color *= 0.75 + 0.25 * sin(uv.y * 320.0);

    // Film grain / static.
    float n = fract(sin(dot(uv * 1000.0, vec2(12.9898, 78.233))) * 43758.5453);
    color += n * 0.08;

    // Vignette (darkened corners/edges).
    color *= 1.0 - dot(uv - 0.5, uv - 0.5) * 0.6;

    // A soft rolling "tracking" band.
    float roll = fract(uv.y * 24.0);
    float track = smoothstep(0.46, 0.5, roll) * smoothstep(0.54, 0.5, roll);
    color = mix(color, vec3(0.85, 0.85, 0.95), track * 0.12);

    // Chromatic fringing — red/blue pull toward the horizontal edges.
    float edge = abs(uv.x - 0.5) * 2.0;
    color.r *= 1.0 + edge * 0.03;
    color.b *= 1.0 - edge * 0.03;

    gl_FragColor = vec4(color, v_fragmentColor.a);
}
)";

// Builds (once) and caches the shared VHS overlay shader program.
static cocos2d::CCGLProgram* vhsOverlayProgram() {
    static cocos2d::CCGLProgram* program = [] {
        auto* p = new cocos2d::CCGLProgram();
        p->initWithVertexShaderByteArray(VHS_OVERLAY_VERTEX, VHS_OVERLAY_FRAGMENT);
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
// The VHS Overlay object
// ---------------------------------------------------------------------------

class $object(VHSOverlay, GameObject) {
public:
    // Custom property key (must not collide with GD's built-in keys for
    // GameObject; 150 is free for plain GameObjects).
    static constexpr size_t OPACITY_KEY = 150;

    // Default translucency: 100/255 (~39% opaque).
    static constexpr int DEFAULT_OPACITY = 100;

    static VHSOverlay* create(ObjectInfo* info) {
        return new VHSOverlay(info);
    }

    VHSOverlay(ObjectInfo* info)
        : CustomObject(info, ObjectTraits::builder()
            .gameObjectType(GameObjectType::Decoration)
            .build()) {}

    // Opacity of the full-screen overlay (0 = invisible, 255 = opaque).
    int m_opacity = DEFAULT_OPACITY;

    // Shared overlay node — only one VHS overlay is shown at a time, no matter
    // how many VHS Overlay objects are in the level.
    inline static cocos2d::CCSprite* s_overlay = nullptr;

    void postPlayLayerInit() override {
        auto* playLayer = PlayLayer::get();
        if (!playLayer) return;

        // Clean up any previous overlay (level restart / multiple objects).
        if (s_overlay) {
            s_overlay->removeFromParentAndCleanup(true);
            s_overlay = nullptr;
        }

        // The marker object itself is editor-only; hide it during gameplay.
        this->setVisible(false);

        // A solid white square scaled to cover the whole window. The texture
        // is irrelevant (the shader synthesizes the effect), it just needs to
        // be a screen-sized quad with standard UVs.
        auto* overlay = CCSprite::create("square02_001.png");
        const CCSize winSize = CCDirector::sharedDirector()->getWinSize();
        overlay->setScaleX(winSize.width / overlay->getContentSize().width);
        overlay->setScaleY(winSize.height / overlay->getContentSize().height);
        overlay->setAnchorPoint({ 0.f, 0.f });
        overlay->setPosition({ 0.f, 0.f });
        overlay->setOpacity(static_cast<GLubyte>(std::clamp(m_opacity, 0, 255)));
        overlay->setShaderProgram(vhsOverlayProgram());
        overlay->setID("vhs-overlay"_spr);

        // Above gameplay, below the pause/HUD layers.
        playLayer->addChild(overlay, 10000);
        s_overlay = overlay;
    }

    static PopupOptions getEditSpecialConfig(const Selected& selected) {
        return PopupConfig::builder()
            .width(260)
            .height(130)
            .title("VHS Overlay")
            .info(InfoPopup::builder()
                .title("VHS Overlay Help")
                .description(
                    "A full-screen VHS-style effect: scanlines, film grain, "
                    "vignette and color fringing. Opacity controls how "
                    "strongly the overlay shows over the level.")
                .build())
            .menu(NumericMenu::builder()
                .id("opacity"_spr)
                .title("Opacity")
                .inputType(NumericMenu::InputType::Slider)
                .min(0)
                .max(255)
                .precision(0)
                .onValue([](const int value, const Selected& selected, Popup* popup) {
                    applyValueToSelected(selected, &VHSOverlay::m_opacity, value);
                })
                .currentValue([](const Selected& selected, Popup* popup) {
                    return getCommonValueOrDefault(selected, &VHSOverlay::m_opacity);
                })
                .build())
            .build();
    }

    std::vector<std::string> getObjectDetails() override {
        return DetailsBuilder::builder()
            .field("Effect", "VHS Overlay")
            .field("Opacity", m_opacity)
            .build();
    }
};

// ---------------------------------------------------------------------------
// PlayLayer hook — drop the shared overlay reference on level exit so it can't
// dangle (the overlay itself is cleaned up with the PlayLayer).
// ---------------------------------------------------------------------------

class $modify(VHSPlayLayer, PlayLayer) {
    void onExit() {
        PlayLayer::onExit();
        VHSOverlay::s_overlay = nullptr;
    }
};

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------

$on_mod(Loaded) {
    ObjectAPI::registerObject(ObjectInfo::builder()
        .id("vhs-overlay"_spr)
        .sprite("vhs.png"_spr)
        .editorTab(EditorTab::Decorations)
        .editorButtonColor(EditorButtonColor::Pink)
        .construction(ComplexObject::builder()
            .factory(VHSOverlay::create)
            .customProperties({
                PropertyInterface::from(VHSOverlay::OPACITY_KEY, &VHSOverlay::m_opacity, VHSOverlay::DEFAULT_OPACITY),
            })
            .build())
        .editSpecial(VHSOverlay::getEditSpecialConfig)
        .build());
}
