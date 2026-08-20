// Cynth's Objects — a pack of custom objects for Geometry Dash (Geode).
//
// Built on top of Object Collab (smjs.object-collab), which provides the
// custom-object framework and cross-mod compatibility layer.
//
// The "Advanced Options Trigger" lets level makers change any level setting
// on the fly — gameplay mode, physics fixes, camera behavior, visual
// toggles, and more.  Each trigger instance changes ONE setting to a
// specific value when activated.

#include <algorithm>

#include <Geode/Geode.hpp>
#include <smjs.object-collab/include/object_collab.hpp>

using namespace geode::prelude;
using namespace object_collab::prelude;

// ---------------------------------------------------------------------------
// Advanced Options Trigger
// ---------------------------------------------------------------------------

// Every setting the trigger can change, with its category and value type.
//
// Bool settings:   value <= 0.5 → false,  value > 0.5 → true
// Speed settings:  value 0→1 maps to Speed 0→4 (slow/normal/fast/very fast/extreme)
// Mode settings:   value 0→1 maps to GameMode 0→7 (cube/ship/ball/ufo/wave/robot/spider/swing)
// Int settings:    value 0→1 maps to the integer range documented below
//
// ★ = takes effect immediately
// ⚡ = takes effect on next respawn / level restart
enum class AdvancedOption : int {
    // ── Gameplay ──────────────────────────────────────────────────────
    PlatformerMode      =  0,  // ★  bool   toggle platformer mode
    MirrorMode          =  1,  // ⚡  bool   mirror all gameplay
    RotateGameplay      =  2,  // ⚡  bool   rotate gameplay 90°
    TwoPlayerMode       =  3,  // ⚡  bool   two-player mode
    ReverseGameplay     =  4,  // ⚡  bool   reverse level direction
    StartMini           =  5,  // ⚡  bool   start as mini
    StartDual           =  6,  // ⚡  bool   start as dual
    PracticeMode        =  7,  // ★  bool   toggle practice mode
    PracticeMusicSync   =  8,  // ★  bool   sync music in practice
    IgnoreDamage        =  9,  // ★  bool   noclip / ignore all damage
    DualTouchTrigger    = 10,  // ★  bool   dual-touch trigger mode
    NoTimePenalty       = 11,  // ★  bool   disable time penalty
    StartSpeed          = 12,  // ⚡  speed  starting speed (0=slow→4=extreme)
    StartMode           = 13,  // ⚡  mode   starting gamemode (0=cube→7=swing)
    FreezePlayer        = 14,  // ★  bool   freeze player controls

    // ── Camera ────────────────────────────────────────────────────────
    StaticCameraShake   = 20,  // ★  bool   disable camera shake
    SkipCameraShake     = 21,  // ★  bool   skip all camera shake
    FreezeStartCamera   = 22,  // ★  bool   freeze camera at start
    ResetCamera         = 23,  // ★  bool   reset camera on trigger

    // ── Physics / Bugfixes ────────────────────────────────────────────
    FixGravityBug       = 30,  // ★  bool   fix gravity bug
    FixNegativeScale    = 31,  // ★  bool   fix negative scale
    FixRobotJump        = 32,  // ★  bool   fix robot jump
    FixRadiusCollision  = 33,  // ★  bool   fix radius collision
    DynamicLevelHeight  = 34,  // ★  bool   dynamic level height
    AllowMultiRotation  = 35,  // ★  bool   allow multi-rotation
    EnablePlayerSqueeze = 36,  // ★  bool   enable player squeeze
    Enable22Changes     = 37,  // ★  bool   enable 2.2 changes
    AllowStaticRotate   = 38,  // ★  bool   allow static rotate
    ReverseSync         = 39,  // ★  bool   reverse sync
    DecreaseBoostSlide  = 40,  // ★  bool   decrease boost slide
    EnableImpulseFix    = 41,  // ★  bool   enable impulse fix

    // ── Visual / Debug ────────────────────────────────────────────────
    LowDetailMode       = 50,  // ★  bool   low detail mode
    PortalIndicators    = 51,  // ★  bool   show portal indicators
    OrbIndicators       = 52,  // ★  bool   show orb indicators
    ShowAudioVisualizer = 53,  // ★  bool   show audio visualizer
    DisablePlayerHitbox = 54,  // ★  bool   hide player hitbox
    HitboxesOnDeath     = 55,  // ★  bool   show hitboxes on death
    DebugDraw           = 56,  // ★  bool   enable debug draw mode
};

// Human-readable name for each option.
static const char* optionName(AdvancedOption opt) {
    switch (opt) {
        // Gameplay
        case AdvancedOption::PlatformerMode:      return "Platformer Mode";
        case AdvancedOption::MirrorMode:          return "Mirror Mode";
        case AdvancedOption::RotateGameplay:      return "Rotate Gameplay";
        case AdvancedOption::TwoPlayerMode:       return "Two-Player Mode";
        case AdvancedOption::ReverseGameplay:     return "Reverse Gameplay";
        case AdvancedOption::StartMini:           return "Start Mini";
        case AdvancedOption::StartDual:           return "Start Dual";
        case AdvancedOption::PracticeMode:        return "Practice Mode";
        case AdvancedOption::PracticeMusicSync:   return "Practice Music Sync";
        case AdvancedOption::IgnoreDamage:        return "Ignore Damage";
        case AdvancedOption::DualTouchTrigger:    return "Dual-Touch Trigger";
        case AdvancedOption::NoTimePenalty:       return "No Time Penalty";
        case AdvancedOption::StartSpeed:          return "Start Speed";
        case AdvancedOption::StartMode:           return "Start Game Mode";
        case AdvancedOption::FreezePlayer:        return "Freeze Player";
        // Camera
        case AdvancedOption::StaticCameraShake:   return "Static Camera Shake";
        case AdvancedOption::SkipCameraShake:     return "Skip Camera Shake";
        case AdvancedOption::FreezeStartCamera:   return "Freeze Start Camera";
        case AdvancedOption::ResetCamera:         return "Reset Camera";
        // Physics
        case AdvancedOption::FixGravityBug:       return "Fix Gravity Bug";
        case AdvancedOption::FixNegativeScale:    return "Fix Negative Scale";
        case AdvancedOption::FixRobotJump:        return "Fix Robot Jump";
        case AdvancedOption::FixRadiusCollision:  return "Fix Radius Collision";
        case AdvancedOption::DynamicLevelHeight:  return "Dynamic Level Height";
        case AdvancedOption::AllowMultiRotation:  return "Allow Multi-Rotation";
        case AdvancedOption::EnablePlayerSqueeze: return "Enable Player Squeeze";
        case AdvancedOption::Enable22Changes:     return "Enable 2.2 Changes";
        case AdvancedOption::AllowStaticRotate:   return "Allow Static Rotate";
        case AdvancedOption::ReverseSync:         return "Reverse Sync";
        case AdvancedOption::DecreaseBoostSlide:  return "Decrease Boost Slide";
        case AdvancedOption::EnableImpulseFix:    return "Enable Impulse Fix";
        // Visual
        case AdvancedOption::LowDetailMode:       return "Low Detail Mode";
        case AdvancedOption::PortalIndicators:    return "Portal Indicators";
        case AdvancedOption::OrbIndicators:       return "Orb Indicators";
        case AdvancedOption::ShowAudioVisualizer: return "Show Audio Visualizer";
        case AdvancedOption::DisablePlayerHitbox: return "Disable Player Hitbox";
        case AdvancedOption::HitboxesOnDeath:     return "Hitboxes on Death";
        case AdvancedOption::DebugDraw:           return "Debug Draw";
    }
    return "???";
}

// Value type for each option (determines how the 0–1 slider is interpreted).
enum class OptionType { Bool, Speed, Mode };

static OptionType optionType(AdvancedOption opt) {
    switch (opt) {
        case AdvancedOption::StartSpeed:  return OptionType::Speed;
        case AdvancedOption::StartMode:   return OptionType::Mode;
        default:                          return OptionType::Bool;
    }
}

// Maps the 0–1 slider value to the actual field value.
static float mapValueToFloat(AdvancedOption opt, float slider) {
    switch (optionType(opt)) {
        case OptionType::Speed: return std::clamp(std::floor(slider * 5.0f), 0.0f, 4.0f);
        case OptionType::Mode:  return std::clamp(std::floor(slider * 8.0f), 0.0f, 7.0f);
        case OptionType::Bool:  return (slider > 0.5f) ? 1.0f : 0.0f;
    }
    return slider;
}

// Formats the value for display.
static std::string formatValue(AdvancedOption opt, float slider) {
    float v = mapValueToFloat(opt, slider);
    switch (optionType(opt)) {
        case OptionType::Bool:  return (v > 0.5f) ? "ON" : "OFF";
        case OptionType::Speed: {
            static const char* names[] = {"Slow", "Normal", "Fast", "V.Fast", "Extreme"};
            int idx = static_cast<int>(v);
            return (idx >= 0 && idx < 5) ? names[idx] : std::to_string(v);
        }
        case OptionType::Mode: {
            static const char* names[] = {"Cube", "Ship", "Ball", "UFO", "Wave", "Robot", "Spider", "Swing"};
            int idx = static_cast<int>(v);
            return (idx >= 0 && idx < 8) ? names[idx] : std::to_string(v);
        }
    }
    return std::to_string(v);
}

class $object(AdvancedOptionsTrigger, EffectGameObject) {
public:
    static constexpr size_t KEY_OPTION = 150;
    static constexpr size_t KEY_VALUE  = 151;

    float m_option = 0.0f;   // AdvancedOption enum value (stored as float for slider compat)
    float m_value  = 1.0f;   // 0–1 slider, interpreted by optionType

    static AdvancedOptionsTrigger* create(ObjectInfo* info) {
        return new AdvancedOptionsTrigger(info);
    }

    AdvancedOptionsTrigger(ObjectInfo* info)
        : CustomObject(info, ObjectTraits::builder()
            .gameObjectType(GameObjectType::Modifier)
            .isStoppableTrigger(false)
            .build()) {}

    void triggerObject(GJBaseGameLayer* layer, int uniqueID,
                       const gd::vector<int>* remapKeys) override {
        applyOption();
        CustomObject::triggerObject(layer, uniqueID, remapKeys);
    }

    void triggerActivated(float spawnXPosition) override {
        applyOption();
        CustomObject::triggerActivated(spawnXPosition);
    }

    void postEditorInit() override {
        this->setTriggerText("ADV OPT");
    }

    // ── Apply the selected option to the running level ────────────────

    void applyOption() {
        auto* pl = PlayLayer::get();
        if (!pl) {
            log::warn("[AdvOpt] No PlayLayer — option not applied");
            return;
        }

        auto* settings = pl->m_levelSettings;
        AdvancedOption opt = static_cast<AdvancedOption>(static_cast<int>(m_option));
        float raw = mapValueToFloat(opt, m_value);
        bool  b   = (raw > 0.5f);
        int   i   = static_cast<int>(raw);

        log::info("[AdvOpt] Applying {} = {} (raw={})",
                  optionName(opt), formatValue(opt, m_value), raw);

        switch (opt) {
            // ── Gameplay ──────────────────────────────────────────────
            case AdvancedOption::PlatformerMode:
                pl->m_isPlatformer = b;
                if (settings) settings->m_platformerMode = b;
                break;
            case AdvancedOption::MirrorMode:
                if (settings) settings->m_mirrorMode = b;
                break;
            case AdvancedOption::RotateGameplay:
                if (settings) settings->m_rotateGameplay = b;
                break;
            case AdvancedOption::TwoPlayerMode:
                if (settings) settings->m_twoPlayerMode = b;
                break;
            case AdvancedOption::ReverseGameplay:
                if (settings) settings->m_reverseGameplay = b;
                break;
            case AdvancedOption::StartMini:
                if (settings) settings->m_startMini = b;
                break;
            case AdvancedOption::StartDual:
                if (settings) settings->m_startDual = b;
                break;
            case AdvancedOption::PracticeMode:
                pl->m_isPracticeMode = b;
                break;
            case AdvancedOption::PracticeMusicSync:
                pl->m_practiceMusicSync = b;
                break;
            case AdvancedOption::IgnoreDamage:
                pl->m_ignoreDamage = b;
                break;
            case AdvancedOption::DualTouchTrigger:
                pl->m_dualTouchTrigger = b;
                break;
            case AdvancedOption::NoTimePenalty:
                if (settings) settings->m_noTimePenalty = b;
                break;
            case AdvancedOption::StartSpeed:
                if (settings) settings->m_startSpeed = static_cast<Speed>(i);
                break;
            case AdvancedOption::StartMode:
                if (settings) settings->m_startMode = i;
                break;
            case AdvancedOption::FreezePlayer:
                pl->m_clickBetweenSteps = b;
                pl->m_clickOnSteps = b;
                break;

            // ── Camera ────────────────────────────────────────────────
            case AdvancedOption::StaticCameraShake:
                pl->m_staticCameraShake = b;
                break;
            case AdvancedOption::SkipCameraShake:
                pl->m_skipCameraShake = b;
                break;
            case AdvancedOption::FreezeStartCamera:
                pl->m_freezeStartCamera = b;
                break;
            case AdvancedOption::ResetCamera:
                if (settings) settings->m_resetCamera = b;
                break;

            // ── Physics / Bugfixes ────────────────────────────────────
            case AdvancedOption::FixGravityBug:
                if (settings) settings->m_fixGravityBug = b;
                break;
            case AdvancedOption::FixNegativeScale:
                pl->m_fixNegativeScale = b;
                if (settings) settings->m_fixNegativeScale = b;
                break;
            case AdvancedOption::FixRobotJump:
                if (settings) settings->m_fixRobotJump = b;
                break;
            case AdvancedOption::FixRadiusCollision:
                if (settings) settings->m_fixRadiusCollision = b;
                break;
            case AdvancedOption::DynamicLevelHeight:
                if (settings) settings->m_dynamicLevelHeight = b;
                break;
            case AdvancedOption::AllowMultiRotation:
                if (settings) settings->m_allowMultiRotation = b;
                break;
            case AdvancedOption::EnablePlayerSqueeze:
                if (settings) settings->m_enablePlayerSqueeze = b;
                break;
            case AdvancedOption::Enable22Changes:
                pl->m_enable22Changes = b;
                if (settings) settings->m_enable22Changes = b;
                break;
            case AdvancedOption::AllowStaticRotate:
                pl->m_allowStaticRotate = b;
                if (settings) settings->m_allowStaticRotate = b;
                break;
            case AdvancedOption::ReverseSync:
                if (settings) settings->m_reverseSync = b;
                break;
            case AdvancedOption::DecreaseBoostSlide:
                if (settings) settings->m_decreaseBoostSlide = b;
                break;
            case AdvancedOption::EnableImpulseFix:
                if (settings) settings->m_enableImpulseFix = b;
                break;

            // ── Visual / Debug ────────────────────────────────────────
            case AdvancedOption::LowDetailMode:
                pl->m_lowDetailMode = b;
                break;
            case AdvancedOption::PortalIndicators:
                pl->m_portalIndicators = b;
                break;
            case AdvancedOption::OrbIndicators:
                pl->m_orbIndicators = b;
                break;
            case AdvancedOption::ShowAudioVisualizer:
                pl->m_showAudioVisualizer = b;
                break;
            case AdvancedOption::DisablePlayerHitbox:
                pl->m_disablePlayerHitbox = b;
                break;
            case AdvancedOption::HitboxesOnDeath:
                pl->m_hitboxesOnDeath = b;
                break;
            case AdvancedOption::DebugDraw:
                pl->m_isDebugDrawEnabled = b;
                break;
        }
    }

    // ── Edit Special ──────────────────────────────────────────────────

    static PopupOptions getEditSpecialConfig(const Selected& selected) {
        return PopupConfig::builder()
            .width(400)
            .height(200)
            .title("Advanced Options")
            .info(InfoPopup::builder()
                .title("Advanced Options Trigger")
                .description(
                    "Changes a level setting on the fly when triggered.\n\n"
                    "OPTION KEY: which setting to change (see list below).\n"
                    "VALUE: the new value.\n"
                    "  Bool: 0→0.5 = OFF, 0.5→1 = ON\n"
                    "  Speed: 0=Slow, 0.25=Normal, 0.5=Fast, "
                    "0.75=Very Fast, 1=Extreme\n"
                    "  Mode: 0=Cube, 0.14=Ship, 0.29=Ball, 0.43=UFO, "
                    "0.57=Wave, 0.71=Robot, 0.86=Spider, 1=Swing\n\n"
                    "★ = instant   ⚡ = needs respawn\n\n"
                    "0  ★Platformer Mode   1  ⚡Mirror Mode\n"
                    "2  ⚡Rotate Gameplay   3  ⚡Two-Player\n"
                    "4  ⚡Reverse Gameplay  5  ⚡Start Mini\n"
                    "6  ⚡Start Dual        7  ★Practice Mode\n"
                    "8  ★Practice Sync     9  ★Ignore Damage\n"
                    "10 ★Dual-Touch Trig   11 ★No Time Penalty\n"
                    "12 ⚡Start Speed       13 ⚡Start Mode\n"
                    "14 ★Freeze Player\n"
                    "20 ★Static Cam Shake  21 ★Skip Cam Shake\n"
                    "22 ★Freeze Cam        23 ★Reset Camera\n"
                    "30 ★Fix Gravity       31 ★Fix Neg Scale\n"
                    "32 ★Fix Robot Jump    33 ★Fix Radius\n"
                    "34 ★Dynamic Height    35 ★Multi-Rotation\n"
                    "36 ★Player Squeeze    37 ★2.2 Changes\n"
                    "38 ★Static Rotate     39 ★Reverse Sync\n"
                    "40 ★Decrease Boost    41 ★Impulse Fix\n"
                    "50 ★Low Detail        51 ★Portal Indicators\n"
                    "52 ★Orb Indicators    53 ★Audio Visualizer\n"
                    "54 ★Disable Hitbox    55 ★Hitboxes on Death\n"
                    "56 ★Debug Draw")
                .build())
            .triggerToggles(true)
            .menu(slider("option"_spr, "Option Key", 0.f, 56.f, 1.f,
                         &AdvancedOptionsTrigger::m_option))
            .menu(slider("value"_spr, "Value", 0.f, 1.f, 0.01f,
                         &AdvancedOptionsTrigger::m_value))
            .build();
    }

    std::vector<std::string> getObjectDetails() override {
        AdvancedOption opt = static_cast<AdvancedOption>(static_cast<int>(m_option));
        return DetailsBuilder::builder()
            .field("Setting", optionName(opt))
            .field("Value",   formatValue(opt, m_value))
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
// Registration
// ---------------------------------------------------------------------------

$on_mod(Loaded) {
    ObjectAPI::registerObject(ObjectInfo::builder()
        .id("advanced-options"_spr)
        .sprite("advanced-options.png"_spr)
        .editorTab(EditorTab::Triggers)
        .editorButtonColor(EditorButtonColor::Green)
        .construction(ComplexObject::builder()
            .factory(AdvancedOptionsTrigger::create)
            .customProperties({
                PropertyInterface::from(AdvancedOptionsTrigger::KEY_OPTION, &AdvancedOptionsTrigger::m_option, 0.0f),
                PropertyInterface::from(AdvancedOptionsTrigger::KEY_VALUE,  &AdvancedOptionsTrigger::m_value,  1.0f),
            })
            .build())
        .editSpecial(AdvancedOptionsTrigger::getEditSpecialConfig)
        .build());
}