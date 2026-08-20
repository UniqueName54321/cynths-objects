// Cynth's Objects — a pack of custom objects for Geometry Dash (Geode).
//
// Built on top of Object Collab (smjs.object-collab), which provides the
// custom-object framework and cross-mod compatibility layer.
//
// The "VHS Shader" object is a full-screen post-processing trigger. It
// captures the gameplay into a CCRenderTexture each frame, runs the captured
// image through a VHS-style fragment shader, and composites the result on
// top with configurable fade-in / hold / fade-out timing. Every effect
// parameter (scanlines, grain, chromatic aberration, tracking bands,
// vignette, speed) and every timing parameter (fade in/out, hold, opacity)
// is editable from "Edit Special", mirroring the vanilla shader trigger's
// setup screen.

#include <algorithm>

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <smjs.object-collab/include/object_collab.hpp>

using namespace geode::prelude;
using namespace object_collab::prelude;

// ---------------------------------------------------------------------------
// VHS post-processing shader
// ---------------------------------------------------------------------------

// Vertex shader.
// CC_MVPMatrix is NOT declared here — CCGLProgram injects it automatically
// before your source.  Redeclaring it causes a GLSL compile error.
// Only the attributes and varyings that Cocos doesn't pre-declare go here.
static constexpr const char* VHS_VERTEX = R"(
attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color;

#ifdef GL_ES
varying mediump vec2 v_texCoord;
varying lowp vec4 v_fragmentColor;
#else
varying vec2 v_texCoord;
varying vec4 v_fragmentColor;
#endif

void main() {
    gl_Position = CC_MVPMatrix * a_position;
    v_texCoord  = a_texCoord;
    v_fragmentColor = a_color;
}
)";

// Full-screen VHS fragment shader.  Samples the captured gameplay from
// CC_Texture0 and applies scanlines, grain, chromatic aberration, tracking
// bands, and vignette.
//
// u_time  is driven per-frame by the overlay node; every other u_* uniform
// maps to one of the adjustable sliders.
static constexpr const char* VHS_FRAGMENT = R"(
#ifdef GL_ES
precision mediump float;

varying mediump vec2 v_texCoord;
varying lowp vec4 v_fragmentColor;
#else
varying vec2 v_texCoord;
varying vec4 v_fragmentColor;
#endif

uniform sampler2D CC_Texture0;   // ← captured gameplay texture

uniform float u_time;            // seconds (already multiplied by u_speed)
uniform float u_scanlines;       // 0..1
uniform float u_grain;           // 0..1
uniform float u_chromatic;       // 0..1
uniform float u_tracking;        // 0..1
uniform float u_vignette;        // 0..1

void main() {
    vec2 uv = v_texCoord;

    // ── Chromatic aberration ──────────────────────────────────────────
    // Sample R / G / B channels at horizontally-offset UVs so the effect
    // actually separates the gameplay image, not a white rectangle.
    float chromaOffset = 0.004 * u_chromatic;
    float r = texture2D(CC_Texture0, uv + vec2(chromaOffset, 0.0)).r;
    float g = texture2D(CC_Texture0, uv).g;
    float b = texture2D(CC_Texture0, uv - vec2(chromaOffset, 0.0)).b;
    vec3 color = vec3(r, g, b);

    // ── Horizontal scanlines ──────────────────────────────────────────
    float scan = 0.72 + 0.28 * sin(uv.y * 380.0);
    color *= mix(1.0, scan, u_scanlines);

    // ── Film grain / static (animated) ────────────────────────────────
    float n = fract(sin(dot(uv * 1200.0 + u_time * 73.0,
                            vec2(12.9898, 78.233))) * 43758.5453);
    color += (n - 0.5) * 0.18 * u_grain;

    // ── Vignette (darkened corners / edges) ───────────────────────────
    float vignette = dot(uv - 0.5, uv - 0.5);
    color *= 1.0 - vignette * 0.65 * u_vignette;

    // ── Tracking distortion band (animated) ───────────────────────────
    float roll  = fract(uv.y * 28.0 - u_time * 1.6);
    float track = smoothstep(0.44, 0.50, roll)
                * (1.0 - smoothstep(0.50, 0.56, roll));
    color = mix(color, vec3(0.88, 0.88, 0.96), track * 0.14 * u_tracking);

    // ── Honour sprite opacity (v_fragmentColor.a carries setOpacity) ─
    float alpha = v_fragmentColor.a;
    gl_FragColor = vec4(color * v_fragmentColor.rgb * alpha, alpha);
}
)";

// 🔧 MAGENTA DEBUGGER — swap VHS_FRAGMENT for this in vhsOverlayProgram()
// to smoke-test trigger → overlay → rendering independently of GLSL math.
// If the screen turns screaming magenta, the pipeline is alive.
// If it doesn't, the problem is trigger wiring or scene placement.
static constexpr const char* MAGENTA_DEBUG_FRAGMENT = R"(
#ifdef GL_ES
precision mediump float;
#endif
varying vec2 v_texCoord;
varying vec4 v_fragmentColor;
void main() {
    gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
}
)";

// ---------------------------------------------------------------------------
// Build & cache the shared GL program
// ---------------------------------------------------------------------------

static cocos2d::CCGLProgram* vhsOverlayProgram() {
    static cocos2d::CCGLProgram* program = []() -> cocos2d::CCGLProgram* {
        auto* p = new cocos2d::CCGLProgram();
        if (!p->initWithVertexShaderByteArray(VHS_VERTEX, VHS_FRAGMENT)) {
            log::error("VHS: initWithVertexShaderByteArray failed");
            log::error("VHS vertex shader log:\n{}", p->vertexShaderLog());
            log::error("VHS fragment shader log:\n{}", p->fragmentShaderLog());
            delete p;
            return nullptr;
        }
        p->addAttribute(kCCAttributeNamePosition,  kCCVertexAttrib_Position);
        p->addAttribute(kCCAttributeNameColor,     kCCVertexAttrib_Color);
        p->addAttribute(kCCAttributeNameTexCoord,  kCCVertexAttrib_TexCoords);
        if (!p->link()) {
            log::error("VHS: shader link failed");
            log::error("VHS program log:\n{}", p->programLog());
            delete p;
            return nullptr;
        }
        p->updateUniforms();
        log::info("VHS: shader program compiled and linked successfully");
        return p;
    }();
    return program;
}

// ---------------------------------------------------------------------------
// VHSOverlayNode — the post-processing overlay
// ---------------------------------------------------------------------------

// A screen-sized CCNode that owns a CCRenderTexture and its display sprite.
//
// Every frame the PlayLayer hook (VHSPlayLayer) calls beginCapture() before
// the normal scene traversal and endCapture() after it.  During the normal
// traversal the overlay is hidden so it never captures itself.  endCapture()
// then draws the captured texture through the VHS fragment shader on top.
//
// The node also runs a fade-in / hold / fade-out state machine that drives
// the display sprite's opacity.
class VHSOverlayNode : public cocos2d::CCNode {
public:
    // ── effect parameters (0..1 unless noted) ─────────────────────────
    float scanlines  = 0.5f;
    float grain      = 0.5f;
    float chromatic  = 0.5f;
    float tracking   = 0.5f;
    float vignette   = 0.5f;
    float speed      = 1.0f;

    // ── timing ────────────────────────────────────────────────────────
    float opacity    = 0.5f;   // 0..1 — peak opacity
    float fadeIn     = 0.5f;   // seconds
    float fadeOut    = 0.5f;   // seconds
    float hold       = 0.0f;   // seconds; 0 = hold until stopped

    // ── render texture & display sprite (public for the PlayLayer hook)
    cocos2d::CCRenderTexture* m_renderTex     = nullptr;
    cocos2d::CCSprite*        m_displaySprite = nullptr;

    // ── singleton ─────────────────────────────────────────────────────
    inline static VHSOverlayNode* s_shared = nullptr;

    static VHSOverlayNode* shared() {
        auto* playLayer = PlayLayer::get();
        if (!playLayer) return nullptr;

        if (!s_shared) {
            auto* overlay = new VHSOverlayNode();
            if (overlay->init()) {
                overlay->autorelease();

                const CCSize winSize =
                    CCDirector::sharedDirector()->getWinSize();

                // ── Create the render texture ─────────────────────────
                overlay->m_renderTex = cocos2d::CCRenderTexture::create(
                    static_cast<int>(winSize.width),
                    static_cast<int>(winSize.height));
                if (!overlay->m_renderTex) {
                    log::error("VHS: CCRenderTexture::create failed");
                    delete overlay;
                    return nullptr;
                }
                overlay->m_renderTex->retain();

                // ── Grab the display sprite & attach the VHS shader ───
                auto* shader = vhsOverlayProgram();
                if (!shader) {
                    log::error("VHS: vhsOverlayProgram failed — "
                               "shader compilation/linking error, "
                               "overlay will not be created");
                    delete overlay;
                    return nullptr;
                }
                overlay->m_displaySprite = overlay->m_renderTex->getSprite();
                overlay->m_displaySprite->setShaderProgram(shader);

                // The render-texture's internal sprite already has
                // scaleY = -1 (to flip the FBO output).  Don't mess
                // with its anchor/position — position the render-texture
                // node itself instead.
                overlay->m_renderTex->setPosition({
                    winSize.width * 0.5f,
                    winSize.height * 0.5f
                });

                // The render texture already owns the sprite as a child;
                // add the render texture as *our* child.
                overlay->addChild(overlay->m_renderTex);

                // ── Cache uniform locations ───────────────────────────
                auto* prog = overlay->m_displaySprite->getShaderProgram();
                if (prog) {
                    overlay->uScanlines  = prog->getUniformLocationForName("u_scanlines");
                    overlay->uGrain      = prog->getUniformLocationForName("u_grain");
                    overlay->uChromatic  = prog->getUniformLocationForName("u_chromatic");
                    overlay->uTracking   = prog->getUniformLocationForName("u_tracking");
                    overlay->uVignette   = prog->getUniformLocationForName("u_vignette");
                    overlay->uTime       = prog->getUniformLocationForName("u_time");
                }

                // ── Position ourselves at the PlayLayer root ──────────
                overlay->setAnchorPoint({0.f, 0.f});
                overlay->setPosition({0.f, 0.f});
                overlay->setContentSize(winSize);
                overlay->setID("vhs-overlay"_spr);
                overlay->setVisible(false);
                overlay->scheduleUpdate();

                playLayer->addChild(overlay, 10000);
                s_shared = overlay;

                log::info("VHS: overlay initialized "
                          "(CCRenderTexture-based post-processing)");
            } else {
                delete overlay;
                return nullptr;
            }
        }
        return s_shared;
    }

    ~VHSOverlayNode() override {
        if (m_renderTex) {
            m_renderTex->release();
            m_renderTex = nullptr;
        }
    }

    // ── public API for the trigger ────────────────────────────────────

    void show() {
        log::info("VHS: showOverlay called");
        m_elapsed       = 0.f;
        m_fadeFromAlpha = 0.f;
        m_fadingOut     = false;
        if (m_displaySprite) {
            m_displaySprite->setOpacity(0);
        }
        setVisible(true);
    }

    void stop() {
        if (isVisible() && !m_fadingOut) {
            m_fadingOut     = true;
            m_elapsed       = 0.f;
            m_fadeFromAlpha = m_displaySprite
                ? m_displaySprite->getOpacity() / 255.0f
                : 0.f;
        }
    }

    // ── capture API (called from VHSPlayLayer::visit) ─────────────────

    // Hide ourselves and start capturing the scene into the render texture.
    // Must be paired with endCapture().
    void beginCapture() {
        setVisible(false);                         // don't capture ourselves
        m_renderTex->beginWithClear(0.f, 0.f, 0.f, 0.f);
    }

    // End the capture, restore us to visible, set shader uniforms, and
    // draw the post-processed sprite on top of the scene.
    void endCapture() {
        m_renderTex->end();
        setVisible(true);

        // ── Guard: bail if the display sprite lost its shader ───────
        auto* program = m_displaySprite->getShaderProgram();
        if (!program) {
            log::error(
                "VHS: display sprite has no shader program — "
                "skipping post-process draw (sprite={})",
                fmt::ptr(m_displaySprite));
            return;
        }

        // ── Push shader uniforms ──────────────────────────────────────
        program->use();
        program->setUniformsForBuiltins();
        if (uScanlines  != -1) program->setUniformLocationWith1f(uScanlines,  scanlines);
        if (uGrain      != -1) program->setUniformLocationWith1f(uGrain,      grain);
        if (uChromatic  != -1) program->setUniformLocationWith1f(uChromatic,  chromatic);
        if (uTracking   != -1) program->setUniformLocationWith1f(uTracking,   tracking);
        if (uVignette   != -1) program->setUniformLocationWith1f(uVignette,   vignette);
        if (uTime       != -1) program->setUniformLocationWith1f(uTime,       m_time * speed);

        // ── Draw the post-processed sprite ────────────────────────────
        // Calling visit() on the render-texture node visits its only
        // child (the display sprite), which draws the captured gameplay
        // through the VHS shader.
        m_renderTex->visit();
    }

    // ── per-frame update (fade state machine) ─────────────────────────

    void update(float dt) override {
        m_time += dt;

        if (!isVisible() || !m_displaySprite) return;

        m_elapsed += dt;

        if (!m_fadingOut) {
            // ── Fade in ───────────────────────────────────────────────
            float fade  = fadeIn > 0.f
                ? std::clamp(m_elapsed / fadeIn, 0.f, 1.f)
                : 1.f;
            float alpha = opacity * fade;

            if (hold > 0.f && m_elapsed > fadeIn + hold) {
                // Transition to fade-out.
                m_fadingOut     = true;
                m_elapsed       = 0.f;
                m_fadeFromAlpha = alpha;
            } else {
                m_displaySprite->setOpacity(toOpacity(alpha));
            }
        } else {
            // ── Fade out ──────────────────────────────────────────────
            float fade  = fadeOut > 0.f
                ? std::clamp(1.f - m_elapsed / fadeOut, 0.f, 1.f)
                : 0.f;
            float alpha = m_fadeFromAlpha * fade;
            m_displaySprite->setOpacity(toOpacity(alpha));

            if (m_elapsed > fadeOut) {
                setVisible(false);
            }
        }
    }

private:
    // Cached uniform locations (-1 = not found).
    GLint uScanlines  = -1;
    GLint uGrain      = -1;
    GLint uChromatic  = -1;
    GLint uTracking   = -1;
    GLint uVignette   = -1;
    GLint uTime       = -1;

    // Shader clock (seconds, driven by update()).
    float m_time = 0.f;

    // Fade state machine.
    float m_elapsed       = 0.f;
    float m_fadeFromAlpha = 0.f;
    bool  m_fadingOut     = false;

    static GLubyte toOpacity(float alpha) {
        return static_cast<GLubyte>(
            std::clamp(alpha, 0.f, 1.f) * 255.0f);
    }
};

// ---------------------------------------------------------------------------
// VHS Shader trigger object
// ---------------------------------------------------------------------------

class $object(VHSShaderTrigger, EffectGameObject) {
public:
    // Custom property keys.  These are free for a plain EffectGameObject
    // (they avoid every key used by GameObject / EnhancedGameObject /
    // EffectGameObject — note 155 & 156 are taken by GameObject).
    static constexpr size_t KEY_OPACITY    = 150;
    static constexpr size_t KEY_FADE_IN    = 151;
    static constexpr size_t KEY_FADE_OUT   = 152;
    static constexpr size_t KEY_HOLD       = 153;
    static constexpr size_t KEY_SCANLINES  = 154;
    static constexpr size_t KEY_GRAIN      = 157;
    static constexpr size_t KEY_CHROMATIC  = 158;
    static constexpr size_t KEY_TRACKING   = 159;
    static constexpr size_t KEY_VIGNETTE   = 160;
    static constexpr size_t KEY_SPEED      = 161;

    // Adjustable parameters.
    float m_opacity   = 0.5f;   // 0..1
    float m_fadeIn    = 0.5f;   // seconds
    float m_fadeOut   = 0.5f;   // seconds
    float m_hold      = 0.0f;   // seconds; 0 = until stopped / level end
    float m_scanlines = 0.5f;   // 0..1
    float m_grain     = 0.5f;   // 0..1
    float m_chromatic = 0.5f;   // 0..1
    float m_tracking  = 0.5f;   // 0..1
    float m_vignette  = 0.5f;   // 0..1
    float m_speed     = 1.0f;   // animation speed multiplier

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

    void triggerObject(GJBaseGameLayer* layer, int uniqueID,
                       const gd::vector<int>* remapKeys) override {
        showOverlay();
        CustomObject::triggerObject(layer, uniqueID, remapKeys);
    }

    void triggerActivated(float spawnXPosition) override {
        showOverlay();
        CustomObject::triggerActivated(spawnXPosition);
    }

    void postEditorInit() override {
        this->setTriggerText("VHS");
    }

    void showOverlay() {
        auto* overlay = VHSOverlayNode::shared();
        if (!overlay) return;

        overlay->opacity   = m_opacity;
        overlay->fadeIn    = m_fadeIn;
        overlay->fadeOut   = m_fadeOut;
        overlay->hold      = m_hold;
        overlay->scanlines = m_scanlines;
        overlay->grain     = m_grain;
        overlay->chromatic = m_chromatic;
        overlay->tracking  = m_tracking;
        overlay->vignette  = m_vignette;
        overlay->speed     = m_speed;

        overlay->show();
    }

    static void stopOverlay() {
        if (VHSOverlayNode::s_shared) {
            VHSOverlayNode::s_shared->stop();
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
                    "A full-screen VHS-style post-processing effect. "
                    "Trigger it to fade the effect in over the level; "
                    "it fades out after the hold time (0 = hold forever) "
                    "or when stopped. Every part of the effect is "
                    "adjustable independently.")
                .build())
            .triggerToggles(true)
            .menu(slider("opacity"_spr,   "Opacity",    0.f, 1.f,  0.01f, &VHSShaderTrigger::m_opacity))
            .menu(slider("fade-in"_spr,   "Fade In",    0.f, 3.f,  0.05f, &VHSShaderTrigger::m_fadeIn))
            .menu(slider("fade-out"_spr,  "Fade Out",   0.f, 3.f,  0.05f, &VHSShaderTrigger::m_fadeOut))
            .menu(slider("hold"_spr,      "Hold",       0.f, 10.f, 0.1f,  &VHSShaderTrigger::m_hold))
            .menu(slider("scanlines"_spr, "Scanlines",  0.f, 1.f,  0.01f, &VHSShaderTrigger::m_scanlines))
            .menu(slider("grain"_spr,     "Grain",      0.f, 1.f,  0.01f, &VHSShaderTrigger::m_grain))
            .menu(slider("chromatic"_spr, "Chromatic",  0.f, 1.f,  0.01f, &VHSShaderTrigger::m_chromatic))
            .menu(slider("tracking"_spr,  "Tracking",   0.f, 1.f,  0.01f, &VHSShaderTrigger::m_tracking))
            .menu(slider("vignette"_spr,  "Vignette",   0.f, 1.f,  0.01f, &VHSShaderTrigger::m_vignette))
            .menu(slider("speed"_spr,     "Speed",      0.f, 4.f,  0.05f, &VHSShaderTrigger::m_speed))
            .build();
    }

    std::vector<std::string> getObjectDetails() override {
        return DetailsBuilder::builder()
            .field("Effect",   "VHS Shader")
            .field("Opacity",  m_opacity)
            .field("Fade in",  m_fadeIn)
            .field("Fade out", m_fadeOut)
            .field("Hold",     m_hold)
            .build();
    }

private:
    template <typename T>
    static std::unique_ptr<NumericMenu> slider(
        std::string id, std::string title,
        float min, float max, float step, float T::* member) {
        return NumericMenu::builder()
            .id(std::move(id))
            .title(std::move(title))
            .inputType(NumericMenu::InputType::Slider)
            .min(min)
            .max(max)
            .stepSize(step)
            .precision(2)
            .onValue([member](const float value, const Selected& selected,
                              Popup* popup) {
                applyValueToSelected(selected, member, value);
            })
            .currentValue([member](const Selected& selected, Popup* popup) {
                return getCommonValueOrDefault(selected, member);
            })
            .build();
    }
};

// ---------------------------------------------------------------------------
// PlayLayer hook
// ---------------------------------------------------------------------------
//
// We intercept PlayLayer::visit() so we can capture the gameplay into a
// CCRenderTexture *before* the VHS overlay sprite draws on top.
//
// Flow per frame when the overlay is active:
//   1. beginCapture() — hide overlay, switch FBO to render texture
//   2. PlayLayer::visit() — game renders into the texture
//   3. endCapture() — restore screen FBO, set shader uniforms, draw the
//      post-processed sprite on top
//
// When the overlay is inactive this hook is a no-op (just passes through to
// the original visit).

class $modify(VHSPlayLayer, PlayLayer) {
    void visit() {
        auto* overlay = VHSOverlayNode::s_shared;
        if (overlay && overlay->isVisible()) {
            overlay->beginCapture();
            PlayLayer::visit();
            overlay->endCapture();
        } else {
            PlayLayer::visit();
        }
    }

    void onExit() {
        PlayLayer::onExit();
        // The overlay is a child of this PlayLayer, so it will be cleaned
        // up automatically.  Null the shared pointer to avoid a dangling
        // reference on the next level load.
        VHSOverlayNode::s_shared = nullptr;
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
                PropertyInterface::from(VHSShaderTrigger::KEY_OPACITY,    &VHSShaderTrigger::m_opacity,   0.5f),
                PropertyInterface::from(VHSShaderTrigger::KEY_FADE_IN,    &VHSShaderTrigger::m_fadeIn,    0.5f),
                PropertyInterface::from(VHSShaderTrigger::KEY_FADE_OUT,   &VHSShaderTrigger::m_fadeOut,   0.5f),
                PropertyInterface::from(VHSShaderTrigger::KEY_HOLD,       &VHSShaderTrigger::m_hold,      0.0f),
                PropertyInterface::from(VHSShaderTrigger::KEY_SCANLINES,  &VHSShaderTrigger::m_scanlines, 0.5f),
                PropertyInterface::from(VHSShaderTrigger::KEY_GRAIN,      &VHSShaderTrigger::m_grain,     0.5f),
                PropertyInterface::from(VHSShaderTrigger::KEY_CHROMATIC,  &VHSShaderTrigger::m_chromatic, 0.5f),
                PropertyInterface::from(VHSShaderTrigger::KEY_TRACKING,   &VHSShaderTrigger::m_tracking,  0.5f),
                PropertyInterface::from(VHSShaderTrigger::KEY_VIGNETTE,   &VHSShaderTrigger::m_vignette,  0.5f),
                PropertyInterface::from(VHSShaderTrigger::KEY_SPEED,      &VHSShaderTrigger::m_speed,     1.0f),
            })
            .build())
        .editSpecial(VHSShaderTrigger::getEditSpecialConfig)
        .build());
}

// ---------------------------------------------------------------------------
// Diagnostic hook — catches any CCSprite reaching draw() with no shader
// ---------------------------------------------------------------------------
//
// This is instrumentation, not a fix.  When a sprite has a null shader
// program, Cocos will crash inside CCSprite::draw() → CCGLProgram::use().
// This hook logs the sprite's identity and returns cleanly instead, so we
// can identify *which* sprite is broken — ours, Blur API's render-texture
// sprite, or another mod entirely.
//
// Once the culprit is identified, this hook should be removed.

#include <Geode/modify/CCSprite.hpp>

class $modify(VHSCCSpriteHook, cocos2d::CCSprite) {
    void draw() {
        if (!getShaderProgram()) {
            auto* parent = getParent();
            log::error(
                "[VHS DIAG] NULL shader — sprite={} id='{}' "
                "parent={} parent-id='{}'",
                fmt::ptr(this), getID(),
                fmt::ptr(parent), parent ? parent->getID() : "<null>");
            return;
        }
        CCSprite::draw();
    }
};