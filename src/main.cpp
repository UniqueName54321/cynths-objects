// Cynth's Objects — a pack of custom objects for Geometry Dash (Geode).
//
// Built on top of Object Collab (smjs.object-collab), which provides the
// custom-object framework and cross-mod compatibility layer.
//
// The "VHS Shader" object is a full-screen shader *trigger*, modeled after
// vanilla GD shader triggers: place it in the level, trigger it (touch /
// spawn / pass-through), and it fades a VHS effect in over the whole screen,
// holds for a configurable time, then fades out. Every part of the effect
// (scanlines, grain, chromatic fringing, tracking bands, vignette, speed) and
// the timing (fade in/out, hold, opacity) is editable from "Edit Special",
// mirroring the vanilla shader trigger's setup screen.

#include <algorithm>

#include <Geode/Geode.hpp>
// `CCGLProgram` normally comes in transitively via <Geode/Geode.hpp>. If your
// compiler can't find it, also include the cocos shader header explicitly:
//   #include <Geode/cocos/shaders/CCGLProgram.h>
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
// `u_time` is driven from a per-frame update on the overlay sprite; every
// other `u_*` uniform is one of the adjustable parameters.
static constexpr const char* VHS_OVERLAY_FRAGMENT = R"(
#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;
varying vec4 v_fragmentColor;

uniform sampler2D CC_Texture0; // declared for cocos2d compatibility (unused)

uniform float u_time;       // seconds (already multiplied by u_speed)
uniform float u_scanlines;  // 0..1
uniform float u_grain;      // 0..1
uniform float u_chromatic;  // 0..1
uniform float u_tracking;   // 0..1
uniform float u_vignette;   // 0..1

void main() {
    vec2 uv = v_texCoord;
    vec3 color = v_fragmentColor.rgb;

    // Horizontal scanlines.
    float scan = 0.75 + 0.25 * sin(uv.y * 320.0);
    color *= mix(1.0, scan, u_scanlines);

    // Film grain / static (animated).
    float n = fract(sin(dot(uv * 1000.0 + u_time * 60.0, vec2(12.9898, 78.233))) * 43758.5453);
    color += (n - 0.5) * 0.16 * u_grain;

    // Vignette (darkened corners/edges).
    color *= 1.0 - dot(uv - 0.5, uv - 0.5) * 0.6 * u_vignette;

    // A soft rolling "tracking" band (animated).
    float roll = fract(uv.y * 24.0 - u_time * 1.5);
    float track = smoothstep(0.46, 0.5, roll) * (1.0 - smoothstep(0.5, 0.54, roll));
    color = mix(color, vec3(0.85, 0.85, 0.95), track * 0.12 * u_tracking);

    // Chromatic fringing — red/blue pull toward the horizontal edges.
    float edge = abs(uv.x - 0.5) * 2.0;
    color.r *= 1.0 + edge * 0.03 * u_chromatic;
    color.b *= 1.0 - edge * 0.03 * u_chromatic;

    gl_FragColor = vec4(color, v_fragmentColor.a);
}
)";

// Builds (once) and caches the shared VHS overlay shader program.
static cocos2d::CCGLProgram* vhsOverlayProgram() {
    static cocos2d::CCGLProgram* program = [] {
        auto* p = new cocos2d::CCGLProgram();
        p->initWithVertexShaderByteArray(VHS_OVERLAY_VERTEX, VHS_OVERLAY_FRAGMENT);
        p->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
        p->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
        p->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
        p->link();
        p->updateUniforms();
        return p;
    }();
    return program;
}

// A tiny 1x1 white texture, generated at runtime so the overlay never depends
// on a specific frame name from GD's game sheet.
static cocos2d::CCTexture2D* whiteSquareTexture() {
    static cocos2d::CCTexture2D* texture = [] {
        auto* tex = new cocos2d::CCTexture2D();
        unsigned char pixel[4] = { 255, 255, 255, 255 };
        tex->initWithData(pixel, cocos2d::kCCTexture2DPixelFormat_RGBA8888, 1, 1,
                          cocos2d::CCSize(1.0f, 1.0f));
        tex->setAntiAliasTexParameters();
        return tex;
    }();
    return texture;
}

// ---------------------------------------------------------------------------
// The full-screen overlay node
// ---------------------------------------------------------------------------

// A screen-sized white quad rendered through the VHS shader. It owns its own
// fade-in / fade-out state machine and advances the shader clock each frame.
class VHSOverlaySprite : public cocos2d::CCSprite {
public:
    // Effect parameters (0..1 unless noted).
    float scanlines = 0.5f;
    float grain = 0.5f;
    float chromatic = 0.5f;
    float tracking = 0.5f;
    float vignette = 0.5f;
    float speed = 1.0f;

    // Timing (seconds). `hold == 0` means "hold until stopped / level end".
    float opacity = 0.5f; // 0..1
    float fadeIn = 0.5f;
    float fadeOut = 0.5f;
    float hold = 0.0f;

    // Shared overlay — only one VHS overlay is shown at a time, no matter how
    // many triggers are in the level.
    inline static VHSOverlaySprite* s_shared = nullptr;

    static VHSOverlaySprite* shared() {
        auto* playLayer = PlayLayer::get();
        if (!playLayer) return nullptr;

        if (!s_shared) {
            auto* overlay = new VHSOverlaySprite();
            if (overlay->initWithTexture(whiteSquareTexture())) {
                overlay->autorelease();
                overlay->setShaderProgram(vhsOverlayProgram());

                // Cache the uniform locations once.
                auto* program = overlay->getShaderProgram();
                overlay->uScanlines = program->getUniformLocationForName("u_scanlines");
                overlay->uGrain = program->getUniformLocationForName("u_grain");
                overlay->uChromatic = program->getUniformLocationForName("u_chromatic");
                overlay->uTracking = program->getUniformLocationForName("u_tracking");
                overlay->uVignette = program->getUniformLocationForName("u_vignette");
                overlay->uTime = program->getUniformLocationForName("u_time");

                const CCSize winSize = CCDirector::sharedDirector()->getWinSize();
                overlay->setScaleX(winSize.width);   // texture is 1x1
                overlay->setScaleY(winSize.height);
                overlay->setAnchorPoint({ 0.f, 0.f });
                overlay->setPosition({ 0.f, 0.f });
                overlay->setID("vhs-overlay"_spr);
                overlay->setVisible(false);
                overlay->scheduleUpdate();

                // Above gameplay, below the pause/HUD layers.
                playLayer->addChild(overlay, 10000);
                s_shared = overlay;
            } else {
                delete overlay;
                return nullptr;
            }
        }

        return s_shared;
    }

    // (Re)starts the fade-in from scratch.
    void show() {
        m_elapsed = 0.f;
        m_fadeFromAlpha = 0.f;
        m_fadingOut = false;
        setOpacity(0);
        setVisible(true);
    }

    // Begins the fade-out phase (used by Stop triggers).
    void stop() {
        if (isVisible() && !m_fadingOut) {
            m_fadingOut = true;
            m_elapsed = 0.f;
            m_fadeFromAlpha = getDisplayedOpacity() / 255.0f;
        }
    }

    // Set the shader's time + parameter uniforms. Called from draw() while the
    // program is bound.
    void draw() override {
        auto* program = getShaderProgram();
        if (program) {
            program->use();
            program->setUniformsForBuiltins();
            if (uScanlines != -1) program->setUniformLocationWith1f(uScanlines, scanlines);
            if (uGrain != -1) program->setUniformLocationWith1f(uGrain, grain);
            if (uChromatic != -1) program->setUniformLocationWith1f(uChromatic, chromatic);
            if (uTracking != -1) program->setUniformLocationWith1f(uTracking, tracking);
            if (uVignette != -1) program->setUniformLocationWith1f(uVignette, vignette);
            if (uTime != -1) program->setUniformLocationWith1f(uTime, m_time * speed);
        }
        CCSprite::draw();
    }

    void update(float dt) override {
        m_time += dt;

        if (!isVisible()) return;

        m_elapsed += dt;

        if (!m_fadingOut) {
            // Fade in.
            float fade = fadeIn > 0.f ? std::clamp(m_elapsed / fadeIn, 0.f, 1.f) : 1.f;
            float alpha = opacity * fade;

            if (hold > 0.f && m_elapsed > fadeIn + hold) {
                m_fadingOut = true;
                m_elapsed = 0.f;
                m_fadeFromAlpha = alpha;
            } else {
                setOpacity(toOpacity(alpha));
            }
        } else {
            // Fade out.
            float fade = fadeOut > 0.f ? std::clamp(1.f - m_elapsed / fadeOut, 0.f, 1.f) : 0.f;
            float alpha = m_fadeFromAlpha * fade;
            setOpacity(toOpacity(alpha));

            if (m_elapsed > fadeOut) {
                setVisible(false);
            }
        }
    }

private:
    // Cached uniform locations.
    GLint uScanlines = -1;
    GLint uGrain = -1;
    GLint uChromatic = -1;
    GLint uTracking = -1;
    GLint uVignette = -1;
    GLint uTime = -1;

    // Shader clock (seconds).
    float m_time = 0.f;

    // Fade state machine.
    float m_elapsed = 0.f;
    float m_fadeFromAlpha = 0.f;
    bool m_fadingOut = false;

    static GLubyte toOpacity(float alpha) {
        return static_cast<GLubyte>(std::clamp(alpha, 0.f, 1.f) * 255.0f);
    }
};

// ---------------------------------------------------------------------------
// The VHS Shader trigger object
// ---------------------------------------------------------------------------

class $object(VHSShaderTrigger, EffectGameObject) {
public:
    // Custom property keys. These are free for a plain EffectGameObject (they
    // avoid every key used by GameObject / EnhancedGameObject /
    // EffectGameObject — note 155 & 156 are taken by GameObject).
    static constexpr size_t KEY_OPACITY = 150;
    static constexpr size_t KEY_FADE_IN = 151;
    static constexpr size_t KEY_FADE_OUT = 152;
    static constexpr size_t KEY_HOLD = 153;
    static constexpr size_t KEY_SCANLINES = 154;
    static constexpr size_t KEY_GRAIN = 157;
    static constexpr size_t KEY_CHROMATIC = 158;
    static constexpr size_t KEY_TRACKING = 159;
    static constexpr size_t KEY_VIGNETTE = 160;
    static constexpr size_t KEY_SPEED = 161;

    // Adjustable parameters.
    float m_opacity = 0.5f;   // 0..1
    float m_fadeIn = 0.5f;    // seconds
    float m_fadeOut = 0.5f;   // seconds
    float m_hold = 0.0f;      // seconds; 0 = until stopped / level end
    float m_scanlines = 0.5f; // 0..1
    float m_grain = 0.5f;     // 0..1
    float m_chromatic = 0.5f; // 0..1
    float m_tracking = 0.5f;  // 0..1
    float m_vignette = 0.5f;  // 0..1
    float m_speed = 1.0f;     // animation speed multiplier

    static VHSShaderTrigger* create(ObjectInfo* info) {
        return new VHSShaderTrigger(info);
    }

    VHSShaderTrigger(ObjectInfo* info)
        : CustomObject(info, ObjectTraits::builder()
            .gameObjectType(GameObjectType::Modifier)
            .isStoppableTrigger(true)
            .onObjectGroupCommand([](GJActionCommand command) {
                if (command == GJActionCommand::Stop) stopOverlay();
            })
            .onControlIDCommand([](GJActionCommand command) {
                if (command == GJActionCommand::Stop) stopOverlay();
            })
            .build()) {}

    void triggerObject(GJBaseGameLayer* layer, int uniqueID, const gd::vector<int>* remapKeys) override {
        showOverlay();
        CustomObject::triggerObject(layer, uniqueID, remapKeys);
    }

    void triggerActivated(float spawnXPosition) override {
        showOverlay();
        CustomObject::triggerActivated(spawnXPosition);
    }

    void postEditorInit() override {
        // Label the trigger in the editor.
        this->setTriggerText("VHS");
    }

    void showOverlay() {
        auto* overlay = VHSOverlaySprite::shared();
        if (!overlay) return;

        overlay->opacity = m_opacity;
        overlay->fadeIn = m_fadeIn;
        overlay->fadeOut = m_fadeOut;
        overlay->hold = m_hold;
        overlay->scanlines = m_scanlines;
        overlay->grain = m_grain;
        overlay->chromatic = m_chromatic;
        overlay->tracking = m_tracking;
        overlay->vignette = m_vignette;
        overlay->speed = m_speed;

        overlay->show();
    }

    static void stopOverlay() {
        if (VHSOverlaySprite::s_shared) {
            VHSOverlaySprite::s_shared->stop();
        }
    }

    static PopupOptions getEditSpecialConfig(const Selected& selected) {
        return PopupConfig::builder()
            .width(360)
            .height(280)
            .title("VHS Shader")
            .info(InfoPopup::builder()
                .title("VHS Shader Help")
                .description(
                    "A full-screen VHS-style shader. Trigger it to fade the "
                    "effect in over the level; it fades out after the hold "
                    "time (0 = hold forever) or when stopped. Every part of "
                    "the effect is adjustable independently.")
                .build())
            .triggerToggles(true)
            .menu(slider("opacity"_spr, "Opacity", 0.f, 1.f, 0.01f, &VHSShaderTrigger::m_opacity))
            .menu(slider("fade-in"_spr, "Fade In", 0.f, 3.f, 0.05f, &VHSShaderTrigger::m_fadeIn))
            .menu(slider("fade-out"_spr, "Fade Out", 0.f, 3.f, 0.05f, &VHSShaderTrigger::m_fadeOut))
            .menu(slider("hold"_spr, "Hold", 0.f, 10.f, 0.1f, &VHSShaderTrigger::m_hold))
            .menu(slider("scanlines"_spr, "Scanlines", 0.f, 1.f, 0.01f, &VHSShaderTrigger::m_scanlines))
            .menu(slider("grain"_spr, "Grain", 0.f, 1.f, 0.01f, &VHSShaderTrigger::m_grain))
            .menu(slider("chromatic"_spr, "Chromatic", 0.f, 1.f, 0.01f, &VHSShaderTrigger::m_chromatic))
            .menu(slider("tracking"_spr, "Tracking", 0.f, 1.f, 0.01f, &VHSShaderTrigger::m_tracking))
            .menu(slider("vignette"_spr, "Vignette", 0.f, 1.f, 0.01f, &VHSShaderTrigger::m_vignette))
            .menu(slider("speed"_spr, "Speed", 0.f, 4.f, 0.05f, &VHSShaderTrigger::m_speed))
            .build();
    }

    std::vector<std::string> getObjectDetails() override {
        return DetailsBuilder::builder()
            .field("Effect", "VHS Shader")
            .field("Opacity", m_opacity)
            .field("Fade in", m_fadeIn)
            .field("Fade out", m_fadeOut)
            .field("Hold", m_hold)
            .build();
    }

private:
    // Builds a numeric slider bound to a float member.
    template <typename T>
    static std::unique_ptr<NumericMenu> slider(
        std::string id, std::string title, float min, float max, float step, float T::* member
    ) {
        return NumericMenu::builder()
            .id(std::move(id))
            .title(std::move(title))
            .inputType(NumericMenu::InputType::Slider)
            .min(min)
            .max(max)
            .stepSize(step)
            .precision(2)
            .onValue([member](const float value, const Selected& selected, Popup* popup) {
                applyValueToSelected(selected, member, value);
            })
            .currentValue([member](const Selected& selected, Popup* popup) {
                return getCommonValueOrDefault(selected, member);
            })
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
        VHSOverlaySprite::s_shared = nullptr;
    }
};

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------

$on_mod(Loaded) {
    ObjectAPI::registerObject(ObjectInfo::builder()
        .id("vhs-shader"_spr)
        .sprite("vhs.png"_spr)
        .editorTab(EditorTab::Triggers)
        .editorButtonColor(EditorButtonColor::Pink)
        .construction(ComplexObject::builder()
            .factory(VHSShaderTrigger::create)
            .customProperties({
                PropertyInterface::from(VHSShaderTrigger::KEY_OPACITY, &VHSShaderTrigger::m_opacity, 0.5f),
                PropertyInterface::from(VHSShaderTrigger::KEY_FADE_IN, &VHSShaderTrigger::m_fadeIn, 0.5f),
                PropertyInterface::from(VHSShaderTrigger::KEY_FADE_OUT, &VHSShaderTrigger::m_fadeOut, 0.5f),
                PropertyInterface::from(VHSShaderTrigger::KEY_HOLD, &VHSShaderTrigger::m_hold, 0.0f),
                PropertyInterface::from(VHSShaderTrigger::KEY_SCANLINES, &VHSShaderTrigger::m_scanlines, 0.5f),
                PropertyInterface::from(VHSShaderTrigger::KEY_GRAIN, &VHSShaderTrigger::m_grain, 0.5f),
                PropertyInterface::from(VHSShaderTrigger::KEY_CHROMATIC, &VHSShaderTrigger::m_chromatic, 0.5f),
                PropertyInterface::from(VHSShaderTrigger::KEY_TRACKING, &VHSShaderTrigger::m_tracking, 0.5f),
                PropertyInterface::from(VHSShaderTrigger::KEY_VIGNETTE, &VHSShaderTrigger::m_vignette, 0.5f),
                PropertyInterface::from(VHSShaderTrigger::KEY_SPEED, &VHSShaderTrigger::m_speed, 1.0f),
            })
            .build())
        .editSpecial(VHSShaderTrigger::getEditSpecialConfig)
        .build());
}
