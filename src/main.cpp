// Cynth's Objects — a pack of custom objects for Geometry Dash (Geode).
//
// Built on top of Object Collab (smjs.object-collab), which provides the
// custom-object framework and cross-mod compatibility layer.
//
// Includes the Advanced Options Trigger plus Combination and Gamemode orbs.
// Trigger configuration lives in Edit Object; orb configuration follows
// vanilla behavior and lives in Edit Special.

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <Geode/ui/SliderNode.hpp>
#include <smjs.object-collab/include/object_collab.hpp>

using namespace geode::prelude;
using namespace object_collab::prelude;

// ---------------------------------------------------------------------------
// Option entry — one row in the option browser
// ---------------------------------------------------------------------------

struct OptionEntry {
    int         key;
    const char* name;
    bool        instant;  // true = ★, false = ⚡
    const char* category;
};

// ── Full option table (alphabetical by category, then key) ────────────

static const OptionEntry g_options[] = {
    // ── Level Settings ────────────────────────────────────────────────
    {  0, "Platformer Mode",          true,  "Level"},
    {  1, "Mirror Mode",              false, "Level"},
    {  2, "Rotate Gameplay",          false, "Level"},
    {  3, "Two-Player Mode",          false, "Level"},
    {  4, "Reverse Gameplay",         false, "Level"},
    {  5, "Start Mini",               false, "Level"},
    {  6, "Start Dual",               false, "Level"},
    {  7, "Start Speed",              false, "Level"},
    {  8, "Start Game Mode",          false, "Level"},
    {  9, "Start w/ Start Pos",       false, "Level"},
    { 10, "Disable Start Pos",        false, "Level"},
    { 11, "No Time Penalty",           true, "Level"},
    { 12, "Reset Camera",              true, "Level"},
    { 13, "Spawn Group",              false, "Level"},
    { 14, "Target Order",             false, "Level"},
    { 15, "Target Channel",           false, "Level"},
    // ── Gameplay ──────────────────────────────────────────────────────
    { 30, "Practice Mode",             true, "Gameplay"},
    { 31, "Practice Music Sync",       true, "Gameplay"},
    { 32, "Ignore Damage",             true, "Gameplay"},
    { 33, "Dual-Touch Trigger",        true, "Gameplay"},
    { 34, "Freeze Player",             true, "Gameplay"},
    { 35, "Audio Paused",              true, "Gameplay"},
    { 36, "Start Optimization",        true, "Gameplay"},
    { 37, "Skip Audio Step",           true, "Gameplay"},
    { 38, "Loading Start Pos",         true, "Gameplay"},
    { 39, "Processing Audio Trig",     true, "Gameplay"},
    { 40, "Keep Group Parents",        true, "Gameplay"},
    { 41, "Reset Active Objects",      true, "Gameplay"},
    { 42, "Skip Art Reload",           true, "Gameplay"},
    { 43, "Update Ground Shadows",     true, "Gameplay"},
    { 44, "Record Inputs",             true, "Gameplay"},
    { 45, "Disable Gravity Effect",    true, "Gameplay"},
    { 46, "Glitter Enabled",           true, "Gameplay"},
    { 47, "BG Effect Disabled",        true, "Gameplay"},
    { 48, "Decimal Percentage",        true, "Gameplay"},
    { 49, "Show Leaderboard %",        true, "Gameplay"},
    { 50, "Song Trigger Interval",     true, "Gameplay"},
    { 51, "Progress Bar Height",       true, "Gameplay"},
    { 52, "Section X Factor",          true, "Gameplay"},
    { 53, "Section Y Factor",          true, "Gameplay"},
    { 54, "Particle Count",            true, "Gameplay"},
    { 55, "Custom Particle Count",     true, "Gameplay"},
    { 56, "Particle System Limit",     true, "Gameplay"},
    // ── Camera ────────────────────────────────────────────────────────
    { 60, "Static Camera Shake",        true, "Camera"},
    { 61, "Skip Camera Shake",          true, "Camera"},
    { 62, "Freeze Start Camera",        true, "Camera"},
    { 63, "Camera Width Offset",        true, "Camera"},
    { 64, "Camera Height Offset",       true, "Camera"},
    { 65, "Camera Unzoomed X",          true, "Camera"},
    { 66, "Half Camera Width",          true, "Camera"},
    { 67, "Unzoomed Height Offset",     true, "Camera"},
    { 68, "Target Height Offset",       true, "Camera"},
    { 69, "Calc Target Height",         true, "Camera"},
    // ── Physics ───────────────────────────────────────────────────────
    { 80, "Fix Gravity Bug",            true, "Physics"},
    { 81, "Fix Negative Scale",         true, "Physics"},
    { 82, "Fix Robot Jump",             true, "Physics"},
    { 83, "Fix Radius Collision",       true, "Physics"},
    { 84, "Dynamic Level Height",       true, "Physics"},
    { 85, "Allow Multi-Rotation",       true, "Physics"},
    { 86, "Enable Player Squeeze",      true, "Physics"},
    { 87, "Enable 2.2 Changes",         true, "Physics"},
    { 88, "Allow Static Rotate",        true, "Physics"},
    { 89, "Reverse Sync",               true, "Physics"},
    { 90, "Decrease Boost Slide",       true, "Physics"},
    { 91, "Enable Impulse Fix",         true, "Physics"},
    // ── Visual/Debug ──────────────────────────────────────────────────
    {100, "Low Detail Mode",            true, "Visual"},
    {101, "Portal Indicators",          true, "Visual"},
    {102, "Orb Indicators",             true, "Visual"},
    {103, "Show Audio Visualizer",      true, "Visual"},
    {104, "Disable Player Hitbox",      true, "Visual"},
    {105, "Hitboxes on Death",          true, "Visual"},
    {106, "Debug Draw",                 true, "Visual"},
    {107, "Level End Anim Started",     true, "Visual"},
    {108, "Objects Deactivated",        true, "Visual"},
    {109, "Active Gradients",           true, "Visual"},
    {110, "Background Index",          false, "Visual"},
    {111, "Ground Index",              false, "Visual"},
    {112, "Font Index",                false, "Visual"},
    {113, "Middle Ground Index",       false, "Visual"},
    {114, "Color Page",                false, "Visual"},
    {115, "Ground Line Index",         false, "Visual"},
    // ── Player ────────────────────────────────────────────────────────
    {130, "Player: Play Effects",       true, "Player"},
    {131, "Player: Ground Particles",   true, "Player"},
    {132, "Player: Ship Particles",     true, "Player"},
    {133, "Player: Robot Anim 1",       true, "Player"},
    {134, "Player: Robot Anim 2",       true, "Player"},
    {135, "Player: Spider Anim",        true, "Player"},
    {136, "Player: Streak Type",        true, "Player"},
    {137, "Player: Streak Width",       true, "Player"},
    {138, "Player: Disable Streak Tint",true, "Player"},
    {139, "Player: Always Show Streak", true, "Player"},
    {140, "Player: Fade Out Streak",    true, "Player"},
    {141, "Player: Switch Wave Color",  true, "Player"},
    {142, "Player: Switch Dash Fire",   true, "Player"},
    {143, "Player: Custom Glow",        true, "Player"},
    {144, "Player: Vehicle Glowing",    true, "Player"},
    {145, "Player: Default Mini Icon",  true, "Player"},
    {146, "Player: Controls Disabled",  true, "Player"},
    {147, "Player: Inputs Locked",      true, "Player"},
    {148, "Player: Practice Death",     true, "Player"},
    {149, "Player: Reduced Effects",    true, "Player"},
    {150, "Player: Quick Checkpoint",   true, "Player"},
    {151, "Player: Can Place Checkpoint",true,"Player"},
    {152, "Player: Affected By Forces", true, "Player"},
    {153, "Player: Trail Particle Life",true, "Player"},
    {154, "Player: Land Particle Angle",true, "Player"},
    {155, "Player: Land Particle Y",    true, "Player"},
    {156, "Player: Use Land Particles", true, "Player"},
    {157, "Player: Is Platformer",      true, "Player"},
    {158, "Player: Ignore Damage",      true, "Player"},
    {159, "Player: Fix Gravity Bug",    true, "Player"},
    {160, "Player: Fix Robot Jump",     true, "Player"},
    {161, "Player: Reverse Sync",       true, "Player"},
    {162, "Player: Decrease Boost",     true, "Player"},
    {163, "Player: Enable 2.2",         true, "Player"},
    {164, "Player: Impulse Fix",        true, "Player"},
    {165, "Player: Disable Squeeze",    true, "Player"},
    {166, "Player: Out Of Bounds",      true, "Player"},
    {167, "Player: Is Second Player",   true, "Player"},
    {168, "Player: Upside Down",        true, "Player"},
    {169, "Player: Going Left",         true, "Player"},
    {170, "Player: Holding Right",      true, "Player"},
    {171, "Player: Holding Left",       true, "Player"},
    {172, "Player: Plat Moving Left",   true, "Player"},
    {173, "Player: Plat Moving Right",  true, "Player"},
    {174, "Player: Plat X Velocity",    true, "Player"},
    {175, "Player: Acceleration",       true, "Player"},
    {176, "Player: X Trajectory",       true, "Player"},
    {177, "Player: Y Trajectory",       true, "Player"},
    {178, "Player: Scale X Time",       true, "Player"},
    {179, "Player: State On Ground",    true, "Player"},
    {180, "Player: State Boost X",      true, "Player"},
    {181, "Player: State Boost Y",      true, "Player"},
    {182, "Player: State Scale",        true, "Player"},
    {183, "Player: State Force",        true, "Player"},
    {184, "Player: State Flip Grav",    true, "Player"},
    {185, "Player: State No Auto Jump", true, "Player"},
    {186, "Player: State Dart Slide",   true, "Player"},
    {187, "Player: State Hit Head",     true, "Player"},
    {188, "Player: Is Accelerating",    true, "Player"},
    {189, "Player: Is Boosted",         true, "Player"},
    {190, "Player: Has Ever Jumped",    true, "Player"},
    {191, "Player: Has Ever Hit Ring",  true, "Player"},
    {192, "Player: Jump Buffered",      true, "Player"},
    {193, "Player: Was Jump Buffered",  true, "Player"},
    {194, "Player: Ring Jump State",    true, "Player"},
    {195, "Player: Touched Ring",       true, "Player"},
    {196, "Player: Touched Custom Ring",true, "Player"},
    {197, "Player: Touched Grav Portal",true, "Player"},
    {198, "Player: Is Colliding",       true, "Player"},
    {199, "Player: Is On Ground",       true, "Player"},
    {200, "Player: Is Falling",         true, "Player"},
    {201, "Player: Sliding Right",      true, "Player"},
    {202, "Player: Is Slope Top",       true, "Player"},
    {203, "Player: Slope Flip Grav",    true, "Player"},
    {204, "Player: Slope Angle",        true, "Player"},
    {205, "Player: Slope Sliding",      true, "Player"},
    {206, "Player: Colliding w/ Slope", true, "Player"},
    {207, "Player: Ball Rotating",      true, "Player"},
    {208, "Player: Ground Material",    true, "Player"},
    {209, "Player: Dash Fire Frame",    true, "Player"},
    {210, "Player: Follow Related",     true, "Player"},
    {211, "Player: Reverse Related",    true, "Player"},
    {212, "Player: Icon Request ID",    true, "Player"},
    {213, "Player: Was Teleported",     true, "Player"},
    {214, "Player: Just Placed Streak", true, "Player"},
    {215, "Player: Pad/Ring Related",   true, "Player"},
    {216, "Player: Can Run Into Block", true, "Player"},
    {217, "Player: Ring Jump Related",  true, "Player"},
    {218, "Player: Sprite Related",     true, "Player"},
    {219, "Player: Has Stopped",        true, "Player"},
    {220, "Player: Correct Slope Dir",  true, "Player"},
};

// ---------------------------------------------------------------------------
// Advanced Options — enum, helpers, value mapping
// ---------------------------------------------------------------------------

enum class AdvancedOption : int {
    PlatformerMode      =  0,  MirrorMode    =  1,  RotateGameplay  =  2,
    TwoPlayerMode       =  3,  ReverseGameplay= 4,  StartMini       =  5,
    StartDual           =  6,  StartSpeed     =  7,  StartMode       =  8,
    StartsWithStartPos  =  9,  DisableStartPos= 10,  NoTimePenalty   = 11,
    ResetCamera         = 12,  SpawnGroup     = 13,  TargetOrder     = 14,
    TargetChannel       = 15,
    PracticeMode        = 30,  PracticeMusicSync=31, IgnoreDamage    = 32,
    DualTouchTrigger    = 33,  FreezePlayer   = 34,  AudioPaused     = 35,
    StartOptimization   = 36,  SkipAudioStep  = 37,  LoadingStartPos = 38,
    ProcessingAudioTrig = 39,  KeepGroupParents=40,  ResetActiveObjects=41,
    SkipArtReload       = 42,  UpdateGroundShadows=43,RecordInputs   = 44,
    DisableGravityEffect= 45,  GlitterEnabled = 46,  BGEffectDisabled= 47,
    DecimalPercentage   = 48,  ShowLeaderboardPct=49, SongTriggerInterval=50,
    ProgressHeight      = 51,  SectionXFactor = 52,  SectionYFactor  = 53,
    ParticleCount       = 54,  CustomParticleCount=55,ParticleSystemLimit=56,
    StaticCameraShake   = 60,  SkipCameraShake= 61,  FreezeStartCamera=62,
    CameraWidthOffset   = 63,  CameraHeightOffset=64,CameraUnzoomedX  = 65,
    HalfCameraWidth     = 66,  UnzoomedHeightOffset=67,TargetHeightOffset=68,
    CalcTargetHeight    = 69,
    FixGravityBug       = 80,  FixNegativeScale= 81, FixRobotJump    = 82,
    FixRadiusCollision  = 83,  DynamicLevelHeight=84, AllowMultiRotation=85,
    EnablePlayerSqueeze = 86,  Enable22Changes = 87,  AllowStaticRotate=88,
    ReverseSync         = 89,  DecreaseBoostSlide=90, EnableImpulseFix= 91,
    LowDetailMode       = 100, PortalIndicators= 101, OrbIndicators   = 102,
    ShowAudioVisualizer = 103, DisablePlayerHitbox=104,HitboxesOnDeath = 105,
    DebugDraw           = 106, LevelEndAnimStarted=107,ObjectsDeactivated=108,
    ActiveGradients     = 109, BackgroundIndex= 110, GroundIndex     = 111,
    FontIndex           = 112, MiddleGroundIndex=113, ColorPage      = 114,
    GroundLineIndex     = 115,
    PlayerPlayEffects   = 130, PlayerGroundParticles=131,PlayerShipParticles=132,
    PlayerRobotAnim1    = 133, PlayerRobotAnim2 = 134, PlayerSpiderAnim= 135,
    PlayerStreakType    = 136, PlayerStreakWidth= 137, PlayerDisableStreakTint=138,
    PlayerAlwaysShowStreak=139,PlayerFadeOutStreak=140,PlayerSwitchWaveColor=141,
    PlayerSwitchDashFire =142, PlayerHasCustomGlow=143,PlayerVehicleGlowing=144,
    PlayerDefaultMiniIcon=145, PlayerControlsDisabled=146,PlayerInputsLocked=147,
    PlayerPracticeDeath = 148, PlayerReducedEffects=149,PlayerQuickCheckpoint=150,
    PlayerCanPlaceCheckpoint=151,PlayerAffectedByForces=152,PlayerTrailParticleLife=153,
    PlayerLandParticlesAngle=154,PlayerLandParticleY=155,PlayerUseLandParticles=156,
    PlayerIsPlatformer  = 157, PlayerIgnoreDamage=158,PlayerFixGravityBug=159,
    PlayerFixRobotJump  = 160, PlayerReverseSync=161,PlayerDecreaseBoost=162,
    PlayerEnable22      = 163, PlayerEnableImpulseFix=164,PlayerDisableSqueeze=165,
    PlayerIsOutOfBounds = 166, PlayerIsSecondPlayer=167,PlayerIsUpsideDown=168,
    PlayerIsGoingLeft   = 169, PlayerHoldingRight=170,PlayerHoldingLeft=171,
    PlayerPlatformerMovingLeft=172,PlayerPlatformerMovingRight=173,PlayerPlatformerXVelocity=174,
    PlayerAcceleration  = 175, PlayerXTrajectory=176,PlayerYTrajectory=177,
    PlayerScaleXTime    = 178, PlayerStateOnGround=179,PlayerStateBoostX=180,
    PlayerStateBoostY   = 181, PlayerStateScale=182,PlayerStateForce=183,
    PlayerStateFlipGrav = 184, PlayerStateNoAutoJump=185,PlayerStateDartSlide=186,
    PlayerStateHitHead  = 187, PlayerIsAccelerating=188,PlayerIsBoosted=189,
    PlayerHasEverJumped = 190, PlayerHasEverHitRing=191,PlayerJumpBuffered=192,
    PlayerWasJumpBuffered=193,PlayerStateRingJump=194,PlayerTouchedRing=195,
    PlayerTouchedCustomRing=196,PlayerTouchedGravityPortal=197,PlayerIsColliding=198,
    PlayerIsOnGround    = 199, PlayerIsFalling=200,PlayerIsSlidingRight=201,
    PlayerIsSlopeTop    = 202, PlayerSlopeFlipGrav=203,PlayerSlopeAngle=204,
    PlayerSlopeSliding  = 205, PlayerCollidingSlope=206,PlayerBallRotating=207,
    PlayerGroundMaterial= 208, PlayerDashFireFrame=209,PlayerFollowRelated=210,
    PlayerReverseRelated= 211, PlayerIconRequestID=212,PlayerWasTeleported=213,
    PlayerJustPlacedStreak=214,PlayerPadRingRelated=215,PlayerCanRunIntoBlocks=216,
    PlayerRingJumpRelated=217,PlayerMaybeSpriteRelated=218,PlayerHasStopped=219,
    PlayerGoingCorrectSlope=220,
};

static const char* optionName(AdvancedOption opt) {
    for (auto& e : g_options) if (e.key == static_cast<int>(opt)) return e.name;
    return "???";
}

enum class OptionType { Bool, Speed, Mode, Int, Float, Int999 };

static OptionType optionType(AdvancedOption opt) {
    switch (opt) {
        case AdvancedOption::StartSpeed:             return OptionType::Speed;
        case AdvancedOption::StartMode:              return OptionType::Mode;
        case AdvancedOption::SpawnGroup:             return OptionType::Int999;
        case AdvancedOption::TargetOrder:            return OptionType::Int999;
        case AdvancedOption::TargetChannel:          return OptionType::Int999;
        case AdvancedOption::SongTriggerInterval:    return OptionType::Float;
        case AdvancedOption::ProgressHeight:         return OptionType::Float;
        case AdvancedOption::SectionXFactor:         return OptionType::Float;
        case AdvancedOption::SectionYFactor:         return OptionType::Float;
        case AdvancedOption::ParticleCount:          return OptionType::Int;
        case AdvancedOption::CustomParticleCount:    return OptionType::Int;
        case AdvancedOption::ParticleSystemLimit:    return OptionType::Int;
        case AdvancedOption::CameraWidthOffset:      return OptionType::Float;
        case AdvancedOption::CameraHeightOffset:     return OptionType::Float;
        case AdvancedOption::CameraUnzoomedX:        return OptionType::Float;
        case AdvancedOption::HalfCameraWidth:        return OptionType::Float;
        case AdvancedOption::UnzoomedHeightOffset:   return OptionType::Float;
        case AdvancedOption::TargetHeightOffset:     return OptionType::Float;
        case AdvancedOption::ActiveGradients:        return OptionType::Int;
        case AdvancedOption::BackgroundIndex:        return OptionType::Int;
        case AdvancedOption::GroundIndex:            return OptionType::Int;
        case AdvancedOption::FontIndex:              return OptionType::Int;
        case AdvancedOption::MiddleGroundIndex:      return OptionType::Int;
        case AdvancedOption::ColorPage:              return OptionType::Int;
        case AdvancedOption::GroundLineIndex:        return OptionType::Int;
        case AdvancedOption::PlayerStreakType:       return OptionType::Int;
        case AdvancedOption::PlayerStreakWidth:      return OptionType::Float;
        case AdvancedOption::PlayerTrailParticleLife:return OptionType::Float;
        case AdvancedOption::PlayerLandParticlesAngle:return OptionType::Float;
        case AdvancedOption::PlayerLandParticleY:    return OptionType::Float;
        case AdvancedOption::PlayerPlatformerXVelocity:return OptionType::Float;
        case AdvancedOption::PlayerAcceleration:     return OptionType::Float;
        case AdvancedOption::PlayerXTrajectory:      return OptionType::Float;
        case AdvancedOption::PlayerYTrajectory:      return OptionType::Float;
        case AdvancedOption::PlayerScaleXTime:       return OptionType::Float;
        case AdvancedOption::PlayerStateOnGround:    return OptionType::Int;
        case AdvancedOption::PlayerStateBoostX:      return OptionType::Int;
        case AdvancedOption::PlayerStateBoostY:      return OptionType::Int;
        case AdvancedOption::PlayerStateScale:       return OptionType::Int;
        case AdvancedOption::PlayerStateForce:       return OptionType::Int;
        case AdvancedOption::PlayerStateFlipGrav:    return OptionType::Int;
        case AdvancedOption::PlayerStateNoAutoJump:  return OptionType::Int;
        case AdvancedOption::PlayerStateDartSlide:   return OptionType::Int;
        case AdvancedOption::PlayerStateHitHead:     return OptionType::Int;
        case AdvancedOption::PlayerSlopeAngle:       return OptionType::Float;
        case AdvancedOption::PlayerGroundMaterial:   return OptionType::Int;
        case AdvancedOption::PlayerDashFireFrame:    return OptionType::Int;
        case AdvancedOption::PlayerFollowRelated:    return OptionType::Int;
        case AdvancedOption::PlayerReverseRelated:   return OptionType::Int;
        case AdvancedOption::PlayerIconRequestID:    return OptionType::Int999;
        default:                                     return OptionType::Bool;
    }
}

static float mapValueToFloat(AdvancedOption opt, float slider) {
    switch (optionType(opt)) {
        case OptionType::Speed:  return std::clamp(std::floor(slider * 5.0f), 0.0f, 4.0f);
        case OptionType::Mode:   return std::clamp(std::floor(slider * 8.0f), 0.0f, 7.0f);
        case OptionType::Int999: return std::clamp(std::floor(slider * 1000.0f), 0.0f, 999.0f);
        case OptionType::Int:    return std::clamp(std::floor(slider * 100.0f), 0.0f, 100.0f);
        case OptionType::Float:  return slider;
        case OptionType::Bool:   return (slider > 0.5f) ? 1.0f : 0.0f;
    }
    return slider;
}

static std::string formatValue(AdvancedOption opt, float slider) {
    float v = mapValueToFloat(opt, slider);
    switch (optionType(opt)) {
        case OptionType::Bool: return (v > 0.5f) ? "ON" : "OFF";
        case OptionType::Speed: {
            static const char* names[] = {"Slow","Normal","Fast","V.Fast","Extreme"};
            int idx = static_cast<int>(v);
            return (idx >= 0 && idx < 5) ? names[idx] : std::to_string(v);
        }
        case OptionType::Mode: {
            static const char* names[] = {"Cube","Ship","Ball","UFO","Wave","Robot","Spider","Swing"};
            int idx = static_cast<int>(v);
            return (idx >= 0 && idx < 8) ? names[idx] : std::to_string(v);
        }
        case OptionType::Int:
        case OptionType::Int999:
            return std::to_string(static_cast<int>(v));
        case OptionType::Float: {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%.2f", v);
            return buf;
        }
    }
    return "???";
}

using OptionValues = std::map<int, float>;

static bool isKnownOption(int key) {
    return std::ranges::any_of(g_options, [key](const OptionEntry& entry) {
        return entry.key == key;
    });
}

static OptionValues decodeOptionValues(std::string_view encoded) {
    OptionValues values;
    if (encoded.empty() || encoded == "-") return values;

    while (!encoded.empty()) {
        const size_t end = encoded.find(';');
        const std::string_view item = encoded.substr(0, end);
        const size_t equals = item.find('=');

        if (equals != std::string_view::npos) {
            auto key = utils::numFromString<int>(item.substr(0, equals));
            auto value = utils::numFromString<float>(item.substr(equals + 1));
            if (key && value) {
                const int parsedKey = std::move(key).unwrap();
                const float parsedValue = std::move(value).unwrap();
                if (isKnownOption(parsedKey)) {
                    values[parsedKey] = std::clamp(parsedValue, 0.0f, 1.0f);
                }
            }
        }

        if (end == std::string_view::npos) break;
        encoded.remove_prefix(end + 1);
    }

    return values;
}

static std::string encodeOptionValues(const OptionValues& values) {
    if (values.empty()) return "-";

    std::string encoded;
    for (const auto& entry : g_options) {
        auto value = values.find(entry.key);
        if (value == values.end()) continue;

        if (!encoded.empty()) encoded += ';';
        encoded += std::to_string(entry.key);
        encoded += '=';
        encoded += utils::numToString(value->second);
    }
    return encoded;
}

static float optionStep(AdvancedOption opt) {
    switch (optionType(opt)) {
        case OptionType::Bool:   return 1.0f;
        case OptionType::Speed:  return 0.25f;
        case OptionType::Mode:   return 1.0f / 7.0f;
        case OptionType::Int:    return 0.01f;
        case OptionType::Int999: return 0.001f;
        case OptionType::Float:  return 0.01f;
    }
    return 0.01f;
}

// Search, enable, and edit any number of settings without closing the popup.
// Rows are attached directly to ScrollLayer's content layer; the previous
// nested menu was culled as one off-screen node, leaving an empty list.
class MultiOptionBrowserPopup : public Popup {
protected:
    std::function<void(const std::string&)> m_onChanged;
    OptionValues m_values;
    TextInput* m_searchInput = nullptr;
    ScrollLayer* m_scrollLayer = nullptr;
    CCLabelBMFont* m_optionLabel = nullptr;
    CCLabelBMFont* m_valueLabel = nullptr;
    CCMenuItemToggler* m_activeToggle = nullptr;
    SliderNode* m_valueSlider = nullptr;
    std::map<int, CCLabelBMFont*> m_rowLabels;
    int m_selectedKey = 0;
    bool m_legacyState = false;
    bool m_updatingControls = false;

    bool init(
        std::string encoded,
        bool legacyState,
        std::function<void(const std::string&)> onChanged) {
        if (!Popup::init(420.0f, 320.0f)) return false;

        m_onChanged = std::move(onChanged);
        m_values = decodeOptionValues(encoded);
        m_legacyState = legacyState;
        if (!m_values.empty()) m_selectedKey = m_values.begin()->first;
        this->setTitle("Browse Options");

        m_searchInput = TextInput::create(360.0f, "Search by name, category, or key...");
        m_searchInput->setPosition({m_size.width / 2.0f, 276.0f});
        m_searchInput->setScale(0.65f);
        m_searchInput->setCallback([this](const std::string& query) {
            this->rebuildList(query);
        });
        m_mainLayer->addChild(m_searchInput);

        m_optionLabel = CCLabelBMFont::create("", "bigFont.fnt");
        m_optionLabel->setScale(0.38f);
        m_optionLabel->setPosition({150.0f, 242.0f});
        m_mainLayer->addChild(m_optionLabel);

        auto* enabledLabel = CCLabelBMFont::create("Enabled", "bigFont.fnt");
        enabledLabel->setScale(0.3f);
        enabledLabel->setPosition({30.0f, 218.0f});
        m_mainLayer->addChild(enabledLabel);

        m_activeToggle = CCMenuItemExt::createTogglerWithStandardSprites(
            0.55f, [this](CCMenuItemToggler* sender) {
                const bool enabled = !sender->isToggled();
                m_legacyState = false;
                if (enabled) {
                    m_values.try_emplace(m_selectedKey, m_valueSlider->getValue());
                } else {
                    m_values.erase(m_selectedKey);
                }
                this->persist();
                this->updateRowLabel(m_selectedKey);
            });
        m_activeToggle->setPosition({69.0f, 218.0f});
        m_buttonMenu->addChild(m_activeToggle);

        m_valueSlider = SliderNode::create([this](SliderNode*, float value) {
            if (m_updatingControls) return;
            m_legacyState = false;
            m_values[m_selectedKey] = value;
            if (!m_activeToggle->isToggled()) m_activeToggle->toggle(true);
            this->updateValueLabel();
            this->updateRowLabel(m_selectedKey);
            this->persist();
        });
        m_valueSlider->setMin(0.0f);
        m_valueSlider->setMax(1.0f);
        m_valueSlider->setScale(0.7f);
        m_valueSlider->setPosition({245.0f, 218.0f});
        m_mainLayer->addChild(m_valueSlider);

        m_valueLabel = CCLabelBMFont::create("", "bigFont.fnt");
        m_valueLabel->setScale(0.32f);
        m_valueLabel->setPosition({370.0f, 218.0f});
        m_mainLayer->addChild(m_valueLabel);

        m_scrollLayer = ScrollLayer::create({390.0f, 185.0f});
        m_scrollLayer->setPosition({15.0f, 18.0f});
        m_scrollLayer->setTouchEnabled(true);
        m_scrollLayer->m_contentLayer->setLayout(ScrollLayer::createDefaultListLayout(1.0f));
        m_mainLayer->addChild(m_scrollLayer);

        this->selectOption(m_selectedKey, false);
        this->rebuildList("");
        return true;
    }

    void persist() {
        if (m_onChanged) m_onChanged(encodeOptionValues(m_values));
    }

    void selectOption(int key, bool enable) {
        const int previous = m_selectedKey;

        if (enable) {
            // An un-migrated trigger still contains the old one-option state.
            // The first browser choice replaces that legacy selection; later
            // row clicks add settings to the new multi-option state.
            if (m_legacyState && key != previous) m_values.clear();
            m_legacyState = false;
            m_values.try_emplace(key, 1.0f);
        }

        m_selectedKey = key;
        const auto opt = static_cast<AdvancedOption>(key);

        m_updatingControls = true;
        auto value = m_values.find(key);
        const bool enabled = value != m_values.end();
        const float sliderValue = enabled ? value->second : 1.0f;
        m_activeToggle->toggle(enabled);
        m_valueSlider->setSnapStep(optionStep(opt));
        m_valueSlider->setValue(sliderValue);
        m_updatingControls = false;

        m_optionLabel->setString(optionName(opt));
        this->updateValueLabel();
        this->updateRowLabel(previous);
        this->updateRowLabel(key);
        if (enable) this->persist();
    }

    void updateValueLabel() {
        const auto opt = static_cast<AdvancedOption>(m_selectedKey);
        m_valueLabel->setString(formatValue(opt, m_valueSlider->getValue()).c_str());
    }

    void updateRowLabel(int key) {
        auto labelIt = m_rowLabels.find(key);
        if (labelIt == m_rowLabels.end()) return;

        const OptionEntry* found = nullptr;
        for (const auto& entry : g_options) {
            if (entry.key == key) {
                found = &entry;
                break;
            }
        }
        if (!found) return;

        const bool enabled = m_values.contains(key);
        labelIt->second->setString(fmt::format("[{}][{}] #{} - {}: {}",
            enabled ? "x" : " ", found->instant ? "*" : "!",
            found->key, found->category, found->name).c_str());
        labelIt->second->setColor(key == m_selectedKey
            ? ccColor3B{255, 255, 120}
            : enabled ? ccColor3B{120, 255, 120} : ccColor3B{255, 255, 255});
    }

    void rebuildList(const std::string& query) {
        m_scrollLayer->m_contentLayer->removeAllChildren();
        m_rowLabels.clear();

        std::string lowered = query;
        std::ranges::transform(lowered, lowered.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });

        const int priority = m_scrollLayer->getTouchPriority() - 1;
        for (const auto& entry : g_options) {
            std::string haystack = fmt::format("{} {} {}", entry.name, entry.category, entry.key);
            std::ranges::transform(haystack, haystack.begin(), [](unsigned char ch) {
                return static_cast<char>(std::tolower(ch));
            });
            if (!lowered.empty() && !haystack.contains(lowered)) continue;

            auto* row = CCNode::create();
            row->setContentSize({385.0f, 22.0f});
            auto* label = CCLabelBMFont::create("", "chatFont.fnt");
            label->setScale(0.35f);
            label->setAnchorPoint({0.0f, 0.5f});
            label->setPosition({5.0f, 11.0f});
            row->addChild(label);

            auto* button = CCMenuItemExt::createSpriteExtra(row,
                [this, key = entry.key](CCMenuItemSpriteExtra*) {
                    this->selectOption(key, true);
                });
            auto* menu = CCMenu::create();
            menu->setPosition({0.0f, 0.0f});
            menu->setContentSize(row->getContentSize());
            menu->setTouchPriority(priority);
            button->setPosition({row->getContentWidth() / 2.0f, row->getContentHeight() / 2.0f});
            menu->addChild(button);

            m_rowLabels[entry.key] = label;
            m_scrollLayer->m_contentLayer->addChild(menu);
            this->updateRowLabel(entry.key);
        }

        m_scrollLayer->m_contentLayer->updateLayout();
        m_scrollLayer->moveToTop();
    }

public:
    static MultiOptionBrowserPopup* create(
        std::string encoded,
        bool legacyState,
        std::function<void(const std::string&)> onChanged) {
        auto* popup = new MultiOptionBrowserPopup();
        if (popup && popup->init(
            std::move(encoded), legacyState, std::move(onChanged))) {
            popup->autorelease();
            return popup;
        }
        delete popup;
        return nullptr;
    }
};

// ---------------------------------------------------------------------------
// The trigger object
// ---------------------------------------------------------------------------

class $object(AdvancedOptionsTrigger, EffectGameObject) {
public:
    static constexpr size_t KEY_OPTION = 150;
    static constexpr size_t KEY_VALUE  = 151;
    static constexpr size_t KEY_OPTIONS = 152;

    float m_option = 0.0f;
    float m_value  = 1.0f;
    std::string m_options;

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
        applyOptions();
        CustomObject::triggerObject(layer, uniqueID, remapKeys);
    }

    void triggerActivated(float spawnXPosition) override {
        applyOptions();
        CustomObject::triggerActivated(spawnXPosition);
    }

    void postEditorInit() override {
        this->setTriggerText("ADV OPT");
    }

    std::string getOptionsForEditor() const {
        if (!m_options.empty()) return m_options;

        OptionValues legacy;
        const int key = static_cast<int>(m_option);
        if (isKnownOption(key)) legacy[key] = std::clamp(m_value, 0.0f, 1.0f);
        return encodeOptionValues(legacy);
    }

    // ── Apply the option ──────────────────────────────────────────────

    void applyOptions() {
        if (m_options.empty()) {
            applyOption(static_cast<AdvancedOption>(static_cast<int>(m_option)), m_value);
            return;
        }

        for (const auto& [key, value] : decodeOptionValues(m_options)) {
            applyOption(static_cast<AdvancedOption>(key), value);
        }
    }

    void applyOption(AdvancedOption opt, float sliderValue) {
        auto* pl = PlayLayer::get();
        if (!pl) { log::warn("[AdvOpt] No PlayLayer"); return; }

        auto* settings = pl->m_levelSettings;
        float raw = mapValueToFloat(opt, sliderValue);
        bool  b   = (raw > 0.5f);
        int   i   = static_cast<int>(raw);

        log::info("[AdvOpt] {} = {}", optionName(opt), formatValue(opt, sliderValue));

        switch (opt) {
            // ── Level Settings ────────────────────────────────────────
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
            case AdvancedOption::StartSpeed:
                if (settings) settings->m_startSpeed = static_cast<Speed>(i);
                break;
            case AdvancedOption::StartMode:
                if (settings) settings->m_startMode = i;
                break;
            case AdvancedOption::StartsWithStartPos:
                if (settings) settings->m_startsWithStartPos = b;
                break;
            case AdvancedOption::DisableStartPos:
                if (settings) settings->m_disableStartPos = b;
                break;
            case AdvancedOption::NoTimePenalty:
                if (settings) settings->m_noTimePenalty = b;
                break;
            case AdvancedOption::ResetCamera:
                if (settings) settings->m_resetCamera = b;
                break;
            case AdvancedOption::SpawnGroup:
                if (settings) settings->m_spawnGroup = i;
                break;
            case AdvancedOption::TargetOrder:
                if (settings) settings->m_targetOrder = i;
                break;
            case AdvancedOption::TargetChannel:
                if (settings) settings->m_targetChannel = i;
                break;
            // ── PlayLayer / Gameplay ──────────────────────────────────
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
            case AdvancedOption::FreezePlayer:
                pl->m_clickBetweenSteps = b;
                pl->m_clickOnSteps = b;
                break;
            case AdvancedOption::AudioPaused:
                pl->m_audioPaused = b;
                break;
            case AdvancedOption::StartOptimization:
                pl->m_startOptimization = b;
                break;
            case AdvancedOption::SkipAudioStep:
                pl->m_skipAudioStep = b;
                break;
            case AdvancedOption::LoadingStartPos:
                pl->m_loadingStartPosition = b;
                break;
            case AdvancedOption::ProcessingAudioTrig:
                pl->m_processingAudioTriggers = b;
                break;
            case AdvancedOption::KeepGroupParents:
                pl->m_keepGroupParents = b;
                break;
            case AdvancedOption::ResetActiveObjects:
                pl->m_resetActiveObjects = b;
                break;
            case AdvancedOption::SkipArtReload:
                pl->m_skipArtReload = b;
                break;
            case AdvancedOption::UpdateGroundShadows:
                pl->m_updateGroundShadows = b;
                break;
            case AdvancedOption::RecordInputs:
                pl->m_recordInputs = b;
                break;
            case AdvancedOption::DisableGravityEffect:
                pl->m_disableGravityEffect = b;
                break;
            case AdvancedOption::GlitterEnabled:
                pl->m_glitterEnabled = b;
                break;
            case AdvancedOption::BGEffectDisabled:
                pl->m_bgEffectDisabled = b;
                break;
            case AdvancedOption::DecimalPercentage:
                pl->m_decimalPercentage = b;
                break;
            case AdvancedOption::ShowLeaderboardPct:
                pl->m_showLeaderboardPercentage = b;
                break;
            case AdvancedOption::SongTriggerInterval:
                pl->m_songTriggerInterval = raw;
                break;
            case AdvancedOption::ProgressHeight:
                pl->m_progressHeight = raw * 200.0f;
                break;
            case AdvancedOption::SectionXFactor:
                pl->m_sectionXFactor = raw * 2.0f;
                break;
            case AdvancedOption::SectionYFactor:
                pl->m_sectionYFactor = raw * 2.0f;
                break;
            case AdvancedOption::ParticleCount:
                pl->m_particleCount = static_cast<int>(raw * 500.0f);
                break;
            case AdvancedOption::CustomParticleCount:
                pl->m_customParticleCount = static_cast<int>(raw * 500.0f);
                break;
            case AdvancedOption::ParticleSystemLimit:
                pl->m_particleSystemLimit = static_cast<int>(raw * 500.0f);
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
            case AdvancedOption::CameraWidthOffset:
                pl->m_cameraWidthOffset = (raw - 0.5f) * 1000.0f;
                break;
            case AdvancedOption::CameraHeightOffset:
                pl->m_cameraHeightOffset = (raw - 0.5f) * 1000.0f;
                break;
            case AdvancedOption::CameraUnzoomedX:
                pl->m_cameraUnzoomedX = raw * 2000.0f;
                break;
            case AdvancedOption::HalfCameraWidth:
                pl->m_halfCameraWidth = raw * 1000.0f;
                break;
            case AdvancedOption::UnzoomedHeightOffset:
                pl->m_cameraUnzoomedHeightOffset = (raw - 0.5f) * 1000.0f;
                break;
            case AdvancedOption::TargetHeightOffset:
                pl->m_targetCameraHeightOffset = (raw - 0.5f) * 1000.0f;
                break;
            case AdvancedOption::CalcTargetHeight:
                pl->m_calculateTargetHeightOffset = b;
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
            case AdvancedOption::LevelEndAnimStarted:
                pl->m_levelEndAnimationStarted = b;
                break;
            case AdvancedOption::ObjectsDeactivated:
                pl->m_objectsDeactivated = b;
                break;
            case AdvancedOption::ActiveGradients:
                pl->m_activeGradients = static_cast<int>(raw * 100.0f);
                break;
            case AdvancedOption::BackgroundIndex:
                if (settings) settings->m_backgroundIndex = static_cast<int>(raw * 99.0f);
                break;
            case AdvancedOption::GroundIndex:
                if (settings) settings->m_groundIndex = static_cast<int>(raw * 99.0f);
                break;
            case AdvancedOption::FontIndex:
                if (settings) settings->m_fontIndex = static_cast<int>(raw * 99.0f);
                break;
            case AdvancedOption::MiddleGroundIndex:
                if (settings) settings->m_middleGroundIndex = static_cast<int>(raw * 99.0f);
                break;
            case AdvancedOption::ColorPage:
                if (settings) settings->m_colorPage = static_cast<int>(raw * 9.0f);
                break;
            case AdvancedOption::GroundLineIndex:
                if (settings) settings->m_groundLineIndex = static_cast<int>(raw * 99.0f);
                break;
            // ── Player Options ────────────────────────────────────────
            case AdvancedOption::PlayerPlayEffects:
                applyToPlayer1([b](PlayerObject* p) { p->m_playEffects = b; });
                break;
            case AdvancedOption::PlayerGroundParticles:
                applyToPlayer1([b](PlayerObject* p) { p->m_hasGroundParticles = b; });
                break;
            case AdvancedOption::PlayerShipParticles:
                applyToPlayer1([b](PlayerObject* p) { p->m_hasShipParticles = b; });
                break;
            case AdvancedOption::PlayerRobotAnim1:
                applyToPlayer1([b](PlayerObject* p) { p->m_robotAnimation1Enabled = b; });
                break;
            case AdvancedOption::PlayerRobotAnim2:
                applyToPlayer1([b](PlayerObject* p) { p->m_robotAnimation2Enabled = b; });
                break;
            case AdvancedOption::PlayerSpiderAnim:
                applyToPlayer1([b](PlayerObject* p) { p->m_spiderAnimationEnabled = b; });
                break;
            case AdvancedOption::PlayerStreakType:
                applyToPlayer1([i](PlayerObject* p) { p->m_playerStreak = i; });
                break;
            case AdvancedOption::PlayerStreakWidth:
                applyToPlayer1([raw](PlayerObject* p) { p->m_streakStrokeWidth = raw * 10.0f; });
                break;
            case AdvancedOption::PlayerDisableStreakTint:
                applyToPlayer1([b](PlayerObject* p) { p->m_disableStreakTint = b; });
                break;
            case AdvancedOption::PlayerAlwaysShowStreak:
                applyToPlayer1([b](PlayerObject* p) { p->m_alwaysShowStreak = b; });
                break;
            case AdvancedOption::PlayerFadeOutStreak:
                applyToPlayer1([b](PlayerObject* p) { p->m_fadeOutStreak = b; });
                break;
            case AdvancedOption::PlayerSwitchWaveColor:
                applyToPlayer1([b](PlayerObject* p) { p->m_switchWaveTrailColor = b; });
                break;
            case AdvancedOption::PlayerSwitchDashFire:
                applyToPlayer1([b](PlayerObject* p) { p->m_switchDashFireColor = b; });
                break;
            case AdvancedOption::PlayerHasCustomGlow:
                applyToPlayer1([b](PlayerObject* p) { p->m_hasCustomGlowColor = b; });
                break;
            case AdvancedOption::PlayerVehicleGlowing:
                applyToPlayer1([b](PlayerObject* p) { p->m_maybeIsVehicleGlowing = b; });
                break;
            case AdvancedOption::PlayerDefaultMiniIcon:
                applyToPlayer1([b](PlayerObject* p) { p->m_defaultMiniIcon = b; });
                break;
            case AdvancedOption::PlayerControlsDisabled:
                applyToPlayer1([b](PlayerObject* p) { p->m_controlsDisabled = b; });
                break;
            case AdvancedOption::PlayerInputsLocked:
                applyToPlayer1([b](PlayerObject* p) { p->m_inputsLocked = b; });
                break;
            case AdvancedOption::PlayerPracticeDeath:
                applyToPlayer1([b](PlayerObject* p) { p->m_practiceDeathEffect = b; });
                break;
            case AdvancedOption::PlayerReducedEffects:
                applyToPlayer1([b](PlayerObject* p) { p->m_maybeReducedEffects = b; });
                break;
            case AdvancedOption::PlayerQuickCheckpoint:
                applyToPlayer1([b](PlayerObject* p) { p->m_quickCheckpointMode = b; });
                break;
            case AdvancedOption::PlayerCanPlaceCheckpoint:
                applyToPlayer1([b](PlayerObject* p) { p->m_canPlaceCheckpoint = b; });
                break;
            case AdvancedOption::PlayerAffectedByForces:
                applyToPlayer1([b](PlayerObject* p) { p->m_affectedByForces = b; });
                break;
            case AdvancedOption::PlayerTrailParticleLife:
                applyToPlayer1([raw](PlayerObject* p) { p->m_trailingParticleLife = raw * 5.0f; });
                break;
            case AdvancedOption::PlayerLandParticlesAngle:
                applyToPlayer1([raw](PlayerObject* p) { p->m_landParticlesAngle = raw * 360.0f; });
                break;
            case AdvancedOption::PlayerLandParticleY:
                applyToPlayer1([raw](PlayerObject* p) { p->m_landParticleRelatedY = (raw - 0.5f) * 100.0f; });
                break;
            case AdvancedOption::PlayerUseLandParticles:
                applyToPlayer1([b](PlayerObject* p) { p->m_useLandParticles0 = b; });
                break;
            case AdvancedOption::PlayerIsPlatformer:
                applyToPlayer1([b](PlayerObject* p) { p->m_isPlatformer = b; });
                break;
            case AdvancedOption::PlayerIgnoreDamage:
                applyToPlayer1([b](PlayerObject* p) { p->m_ignoreDamage = b; });
                break;
            case AdvancedOption::PlayerFixGravityBug:
                applyToPlayer1([b](PlayerObject* p) { p->m_fixGravityBug = b; });
                break;
            case AdvancedOption::PlayerFixRobotJump:
                applyToPlayer1([b](PlayerObject* p) { p->m_fixRobotJump = b; });
                break;
            case AdvancedOption::PlayerReverseSync:
                applyToPlayer1([b](PlayerObject* p) { p->m_reverseSync = b; });
                break;
            case AdvancedOption::PlayerDecreaseBoost:
                applyToPlayer1([b](PlayerObject* p) { p->m_decreaseBoostSlide = b; });
                break;
            case AdvancedOption::PlayerEnable22:
                applyToPlayer1([b](PlayerObject* p) { p->m_enable22Changes = b; });
                break;
            case AdvancedOption::PlayerEnableImpulseFix:
                applyToPlayer1([b](PlayerObject* p) { p->m_enableImpulseFix = b; });
                break;
            case AdvancedOption::PlayerDisableSqueeze:
                applyToPlayer1([b](PlayerObject* p) { p->m_disablePlayerSqueeze = b; });
                break;
            case AdvancedOption::PlayerIsOutOfBounds:
                applyToPlayer1([b](PlayerObject* p) { p->m_isOutOfBounds = b; });
                break;
            case AdvancedOption::PlayerIsSecondPlayer:
                applyToPlayer1([b](PlayerObject* p) { p->m_isSecondPlayer = b; });
                break;
            case AdvancedOption::PlayerIsUpsideDown:
                applyToPlayer1([b](PlayerObject* p) { p->m_isUpsideDown = b; });
                break;
            case AdvancedOption::PlayerIsGoingLeft:
                applyToPlayer1([b](PlayerObject* p) { p->m_isGoingLeft = b; });
                break;
            case AdvancedOption::PlayerHoldingRight:
                applyToPlayer1([b](PlayerObject* p) { p->m_holdingRight = b; });
                break;
            case AdvancedOption::PlayerHoldingLeft:
                applyToPlayer1([b](PlayerObject* p) { p->m_holdingLeft = b; });
                break;
            case AdvancedOption::PlayerPlatformerMovingLeft:
                applyToPlayer1([b](PlayerObject* p) { p->m_platformerMovingLeft = b; });
                break;
            case AdvancedOption::PlayerPlatformerMovingRight:
                applyToPlayer1([b](PlayerObject* p) { p->m_platformerMovingRight = b; });
                break;
            case AdvancedOption::PlayerPlatformerXVelocity:
                applyToPlayer1([raw](PlayerObject* p) { p->m_platformerXVelocity = (raw - 0.5) * 1000.0; });
                break;
            case AdvancedOption::PlayerAcceleration:
                applyToPlayer1([raw](PlayerObject* p) { p->m_accelerationOrSpeed = raw * 500.0; });
                break;
            case AdvancedOption::PlayerXTrajectory:
                applyToPlayer1([raw](PlayerObject* p) { p->m_xVelocityRelated = (raw - 0.5f) * 1000.0f; });
                break;
            case AdvancedOption::PlayerYTrajectory:
                applyToPlayer1([raw](PlayerObject* p) { p->m_yVelocityRelated3 = (raw - 0.5f) * 1000.0f; });
                break;
            case AdvancedOption::PlayerScaleXTime:
                applyToPlayer1([raw](PlayerObject* p) { p->m_scaleXRelatedTime = raw * 10.0; });
                break;
            case AdvancedOption::PlayerStateOnGround:
                applyToPlayer1([i](PlayerObject* p) { p->m_stateOnGround = i; });
                break;
            case AdvancedOption::PlayerStateBoostX:
                applyToPlayer1([i](PlayerObject* p) { p->m_stateBoostX = i; });
                break;
            case AdvancedOption::PlayerStateBoostY:
                applyToPlayer1([i](PlayerObject* p) { p->m_stateBoostY = i; });
                break;
            case AdvancedOption::PlayerStateScale:
                applyToPlayer1([i](PlayerObject* p) { p->m_stateScale = i; });
                break;
            case AdvancedOption::PlayerStateForce:
                applyToPlayer1([i](PlayerObject* p) { p->m_stateForce = i; });
                break;
            case AdvancedOption::PlayerStateFlipGrav:
                applyToPlayer1([i](PlayerObject* p) { p->m_stateFlipGravity = i; });
                break;
            case AdvancedOption::PlayerStateNoAutoJump:
                applyToPlayer1([i](PlayerObject* p) { p->m_stateNoAutoJump = i; });
                break;
            case AdvancedOption::PlayerStateDartSlide:
                applyToPlayer1([i](PlayerObject* p) { p->m_stateDartSlide = i; });
                break;
            case AdvancedOption::PlayerStateHitHead:
                applyToPlayer1([i](PlayerObject* p) { p->m_stateHitHead = i; });
                break;
            case AdvancedOption::PlayerIsAccelerating:
                applyToPlayer1([b](PlayerObject* p) { p->m_isAccelerating = b; });
                break;
            case AdvancedOption::PlayerIsBoosted:
                applyToPlayer1([b](PlayerObject* p) { p->m_maybeIsBoosted = b; });
                break;
            case AdvancedOption::PlayerHasEverJumped:
                applyToPlayer1([b](PlayerObject* p) { p->m_hasEverJumped = b; });
                break;
            case AdvancedOption::PlayerHasEverHitRing:
                applyToPlayer1([b](PlayerObject* p) { p->m_hasEverHitRing = b; });
                break;
            case AdvancedOption::PlayerJumpBuffered:
                applyToPlayer1([b](PlayerObject* p) { p->m_jumpBuffered = b; });
                break;
            case AdvancedOption::PlayerWasJumpBuffered:
                applyToPlayer1([b](PlayerObject* p) { p->m_wasJumpBuffered = b; });
                break;
            case AdvancedOption::PlayerStateRingJump:
                applyToPlayer1([b](PlayerObject* p) { p->m_stateRingJump = b; });
                break;
            case AdvancedOption::PlayerTouchedRing:
                applyToPlayer1([b](PlayerObject* p) { p->m_touchedRing = b; });
                break;
            case AdvancedOption::PlayerTouchedCustomRing:
                applyToPlayer1([b](PlayerObject* p) { p->m_touchedCustomRing = b; });
                break;
            case AdvancedOption::PlayerTouchedGravityPortal:
                applyToPlayer1([b](PlayerObject* p) { p->m_touchedGravityPortal = b; });
                break;
            case AdvancedOption::PlayerIsColliding:
                applyToPlayer1([b](PlayerObject* p) { p->m_maybeIsColliding = b; });
                break;
            case AdvancedOption::PlayerIsOnGround:
                applyToPlayer1([b](PlayerObject* p) { p->m_isOnGround2 = b; });
                break;
            case AdvancedOption::PlayerIsFalling:
                applyToPlayer1([b](PlayerObject* p) { p->m_maybeIsFalling = b; });
                break;
            case AdvancedOption::PlayerIsSlidingRight:
                applyToPlayer1([b](PlayerObject* p) { p->m_isSlidingRight = b; });
                break;
            case AdvancedOption::PlayerIsSlopeTop:
                applyToPlayer1([b](PlayerObject* p) { p->m_isCurrentSlopeTop = b; });
                break;
            case AdvancedOption::PlayerSlopeFlipGrav:
                applyToPlayer1([b](PlayerObject* p) { p->m_slopeFlipGravityRelated = b; });
                break;
            case AdvancedOption::PlayerSlopeAngle:
                applyToPlayer1([raw](PlayerObject* p) { p->m_slopeAngleRadians = raw * 3.14159f; });
                break;
            case AdvancedOption::PlayerSlopeSliding:
                applyToPlayer1([b](PlayerObject* p) { p->m_slopeSlidingMaybeRotated = b; });
                break;
            case AdvancedOption::PlayerCollidingSlope:
                applyToPlayer1([b](PlayerObject* p) { p->m_isCollidingWithSlope = b; });
                break;
            case AdvancedOption::PlayerBallRotating:
                applyToPlayer1([b](PlayerObject* p) { p->m_isBallRotating = b; });
                break;
            case AdvancedOption::PlayerGroundMaterial:
                applyToPlayer1([i](PlayerObject* p) { p->m_groundObjectMaterial = i; });
                break;
            case AdvancedOption::PlayerDashFireFrame:
                applyToPlayer1([i](PlayerObject* p) { p->m_dashFireFrame = i; });
                break;
            case AdvancedOption::PlayerFollowRelated:
                applyToPlayer1([i](PlayerObject* p) { p->m_followRelated = i; });
                break;
            case AdvancedOption::PlayerReverseRelated:
                applyToPlayer1([i](PlayerObject* p) { p->m_reverseRelated = i; });
                break;
            case AdvancedOption::PlayerIconRequestID:
                applyToPlayer1([i](PlayerObject* p) { p->m_iconRequestID = i; });
                break;
            case AdvancedOption::PlayerWasTeleported:
                applyToPlayer1([b](PlayerObject* p) { p->m_wasTeleported = b; });
                break;
            case AdvancedOption::PlayerJustPlacedStreak:
                applyToPlayer1([b](PlayerObject* p) { p->m_justPlacedStreak = b; });
                break;
            case AdvancedOption::PlayerPadRingRelated:
                applyToPlayer1([b](PlayerObject* p) { p->m_padRingRelated = b; });
                break;
            case AdvancedOption::PlayerCanRunIntoBlocks:
                applyToPlayer1([b](PlayerObject* p) { p->m_maybeCanRunIntoBlocks = b; });
                break;
            case AdvancedOption::PlayerRingJumpRelated:
                applyToPlayer1([b](PlayerObject* p) { p->m_ringJumpRelated = b; });
                break;
            case AdvancedOption::PlayerMaybeSpriteRelated:
                applyToPlayer1([b](PlayerObject* p) { p->m_maybeSpriteRelated = b; });
                break;
            case AdvancedOption::PlayerHasStopped:
                applyToPlayer1([b](PlayerObject* p) { p->m_maybeHasStopped = b; });
                break;
            case AdvancedOption::PlayerGoingCorrectSlope:
                applyToPlayer1([b](PlayerObject* p) { p->m_maybeGoingCorrectSlopeDirection = b; });
                break;
        }
    }

    template <typename F>
    void applyToPlayer1(F&& fn) {
        auto* pl = PlayLayer::get();
        if (!pl) return;
        if (pl->m_player1) fn(pl->m_player1);
    }

    // ── Edit Object ───────────────────────────────────────────────────

    static PopupOptions getEditObjectConfig(const Selected& selected) {
        return PopupConfig::builder()
            .width(380)
            .height(150)
            .title("Advanced Options")
            .info(InfoPopup::builder()
                .title("Advanced Options Trigger")
                .description(
                    "Changes a level or player setting on the fly.\n\n"
                    "Open \"Browse Options\" to search, enable, disable, "
                    "and set values for any number of the 172 settings.\n\n"
                    "  Bool: 0->0.5=OFF, 0.5->1=ON\n"
                    "  Speed: 0=Slow, 0.25=Normal, 0.5=Fast, "
                    "0.75=Very Fast, 1=Extreme\n"
                    "  Mode: 0=Cube, 0.14=Ship, 0.29=Ball, "
                    "0.43=UFO, 0.57=Wave, 0.71=Robot, "
                    "0.86=Spider, 1=Swing\n"
                    "  Int/Float: mapped to documented range\n\n"
                    "[*] = instant   [!] = needs respawn/restart")
                .build())
            .triggerToggles(true)
            .menu(browseButton())
            .build();
    }

    std::vector<std::string> getObjectDetails() override {
        const OptionValues values = decodeOptionValues(getOptionsForEditor());
        std::vector<std::string> details;
        details.reserve(values.size() + 1);
        details.push_back(fmt::format("Settings: {}", values.size()));
        for (const auto& [key, value] : values) {
            const auto opt = static_cast<AdvancedOption>(key);
            details.push_back(fmt::format("{}: {}", optionName(opt), formatValue(opt, value)));
        }
        return details;
    }

private:
    // ── "Browse Options" button ───────────────────────────────────────

    static std::unique_ptr<CustomValueMenu> browseButton() {
        return CustomValueMenu::builder()
            .id("browse")
            .title("Options")
            .factory([](const Selected& selected, geode::Popup* popup) -> CCMenu* {
                auto* menu = CCMenu::create();
                menu->setPosition({0.0f, 0.0f});
                menu->setContentSize({300.0f, 34.0f});

                const Selected selectedCopy = selected;
                auto* sprite = ButtonSprite::create(
                    "Browse Options...", 280, true,
                    "bigFont.fnt", "GJ_button_01.png", 32.0f, 0.65f);
                auto* btn = CCMenuItemExt::createSpriteExtra(sprite,
                    [selectedCopy](CCMenuItemSpriteExtra*) {
                        AdvancedOptionsTrigger* current = nullptr;
                        for (auto* obj : selectedCopy) {
                            if (auto* candidate = dynamic_cast<AdvancedOptionsTrigger*>(obj)) {
                                current = candidate;
                                break;
                            }
                        }
                        const std::string initial = current
                            ? current->getOptionsForEditor() : "-";
                        const bool legacyState = current && current->m_options.empty();
                        auto* browser = MultiOptionBrowserPopup::create(
                            initial, legacyState,
                            [selectedCopy](const std::string& encoded) {
                                applyValueToSelected(selectedCopy,
                                    &AdvancedOptionsTrigger::m_options, encoded);
                            });
                        if (browser) browser->show();
                    });
                btn->setPosition({150.0f, 17.0f});

                menu->addChild(btn);
                return menu;
            })
            .build();
    }
};

// ---------------------------------------------------------------------------
// Shared orb registry helpers
// ---------------------------------------------------------------------------

namespace {
    struct VanillaObjectChoice {
        const char* name;
        int objectID;
        GameObjectType type;
    };

    constexpr std::array<VanillaObjectChoice, 10> VANILLA_ORBS {{
        {"Yellow Orb",       36,   GameObjectType::YellowJumpRing},
        {"Blue Orb",         84,   GameObjectType::GravityRing},
        {"Pink Orb",         141,  GameObjectType::PinkJumpRing},
        {"Green Orb",        1022, GameObjectType::GreenRing},
        {"Red Orb",          1330, GameObjectType::RedJumpRing},
        {"Black Orb",        1333, GameObjectType::DropRing},
        {"Dash Orb",         1704, GameObjectType::DashRing},
        {"Gravity Dash Orb", 1751, GameObjectType::GravityDashRing},
        {"Spider Orb",       3004, GameObjectType::SpiderOrb},
        {"Teleport Orb",     3027, GameObjectType::TeleportOrb},
    }};

    constexpr std::array<VanillaObjectChoice, 8> VANILLA_GAMEMODES {{
        {"Cube",   12,   GameObjectType::CubePortal},
        {"Ship",   13,   GameObjectType::ShipPortal},
        {"Ball",   47,   GameObjectType::BallPortal},
        {"UFO",    111,  GameObjectType::UfoPortal},
        {"Wave",   660,  GameObjectType::WavePortal},
        {"Robot",  745,  GameObjectType::RobotPortal},
        {"Spider", 1331, GameObjectType::SpiderPortal},
        {"Swing",  1933, GameObjectType::SwingPortal},
    }};

    bool isOrbType(GameObjectType type) {
        switch (type) {
            case GameObjectType::YellowJumpRing:
            case GameObjectType::PinkJumpRing:
            case GameObjectType::GravityRing:
            case GameObjectType::GreenRing:
            case GameObjectType::RedJumpRing:
            case GameObjectType::DropRing:
            case GameObjectType::CustomRing:
            case GameObjectType::DashRing:
            case GameObjectType::GravityDashRing:
            case GameObjectType::SpiderOrb:
            case GameObjectType::TeleportOrb:
                return true;
            default:
                return false;
        }
    }

    bool isGamemodePortalType(GameObjectType type) {
        switch (type) {
            case GameObjectType::CubePortal:
            case GameObjectType::ShipPortal:
            case GameObjectType::BallPortal:
            case GameObjectType::UfoPortal:
            case GameObjectType::WavePortal:
            case GameObjectType::RobotPortal:
            case GameObjectType::SpiderPortal:
            case GameObjectType::SwingPortal:
                return true;
            default:
                return false;
        }
    }

    std::optional<GameObjectType> registeredObjectType(uint32_t numericID, const ObjectInfo& info) {
        if (const auto* quick = std::get_if<QuickObject>(&info.getConstruction())) {
            return quick->getObjectType();
        }
        if (auto* object = GameObject::createWithKey(static_cast<int>(numericID))) {
            return object->getType();
        }
        return std::nullopt;
    }

    std::string friendlyObjectName(std::string_view id) {
        const size_t slash = id.find_last_of('/');
        std::string name(id.substr(slash == std::string_view::npos ? 0 : slash + 1));
        bool capitalize = true;
        for (char& c : name) {
            if (c == '-' || c == '_') {
                c = ' ';
                capitalize = true;
            } else if (capitalize) {
                c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                capitalize = false;
            }
        }
        return name;
    }

    EnumMenu::EnumAliasList orbChoices() {
        EnumMenu::EnumAliasList choices;
        choices.emplace_back("None", "");
        for (const auto& orb : VANILLA_ORBS) {
            choices.emplace_back(orb.name, fmt::format("v:{}", orb.objectID));
        }
        for (const auto& [numericID, info] : ObjectAPI::getRegister()) {
            const std::string id(info.getID());
            if (id == "cynth.objects/combination-orb") continue;
            if (auto type = registeredObjectType(numericID, info); type && isOrbType(*type)) {
                choices.emplace_back(friendlyObjectName(id), "c:" + id);
            }
        }
        return choices;
    }

    EnumMenu::EnumAliasList gamemodeChoices() {
        EnumMenu::EnumAliasList choices;
        for (const auto& mode : VANILLA_GAMEMODES) {
            choices.emplace_back(mode.name, fmt::format("v:{}", mode.objectID));
        }
        for (const auto& [numericID, info] : ObjectAPI::getRegister()) {
            if (auto type = registeredObjectType(numericID, info); type && isGamemodePortalType(*type)) {
                const std::string id(info.getID());
                choices.emplace_back(friendlyObjectName(id), "c:" + id);
            }
        }
        return choices;
    }

    std::string choiceName(const EnumMenu::EnumAliasList& choices, const std::string& value) {
        for (const auto& choice : choices) {
            if (choice.value == value) return choice.display;
        }
        return value.empty() ? "None" : friendlyObjectName(value);
    }

    std::optional<int> resolveObjectChoice(std::string_view value) {
        if (value.starts_with("v:")) {
            auto number = geode::utils::numFromString<int>(value.substr(2));
            if (number) return std::move(number).unwrap();
            return std::nullopt;
        }
        if (value.starts_with("c:")) {
            if (auto id = ObjectAPI::getCustomObjectNumericID(value.substr(2))) {
                return static_cast<int>(*id);
            }
        }
        return std::nullopt;
    }
}

// ---------------------------------------------------------------------------
// Combination Orb
// ---------------------------------------------------------------------------

class $object(CombinationOrb, EnhancedGameObject) {
public:
    static constexpr size_t KEY_ORBS = 153;
    static constexpr size_t KEY_DELAY = 154;

    std::string m_orbs;
    float m_delay = 0.0f;

    static CombinationOrb* create(ObjectInfo* info) { return new CombinationOrb(info); }

    CombinationOrb(ObjectInfo* info)
        : CustomObject(info, ObjectTraits::builder()
            .gameObjectType(GameObjectType::CustomRing)
            .build()) {}

    static std::array<std::string, 7> decodeSlots(std::string_view encoded) {
        std::array<std::string, 7> slots;
        size_t start = 0;
        for (size_t i = 0; i < slots.size() && start <= encoded.size(); ++i) {
            const size_t end = encoded.find('|', start);
            slots[i] = std::string(encoded.substr(start,
                end == std::string_view::npos ? encoded.size() - start : end - start));
            if (end == std::string_view::npos) break;
            start = end + 1;
        }
        return slots;
    }

    static std::string encodeSlots(const std::array<std::string, 7>& slots) {
        std::string encoded;
        for (size_t i = 0; i < slots.size(); ++i) {
            if (i) encoded.push_back('|');
            encoded += slots[i];
        }
        return encoded;
    }

    void activatedByPlayer(PlayerObject* player) override {
        const auto slots = decodeSlots(m_orbs);
        std::vector<std::string> active;
        for (const auto& slot : slots) if (!slot.empty()) active.push_back(slot);

        auto activate = [position = this->getPosition()](const std::string& choice, PlayerObject* target) {
            if (auto numericID = resolveObjectChoice(choice)) {
                if (auto* proxy = GameObject::createWithKey(*numericID)) {
                    proxy->setPosition(position);
                    proxy->activatedByPlayer(target);
                }
            }
        };

        if (!active.empty() && m_delay <= 0.0f) {
            for (const auto& choice : active) activate(choice, player);
        } else if (!active.empty()) {
            auto* actions = CCArray::create();
            for (size_t i = 0; i < active.size(); ++i) {
                if (i) actions->addObject(CCDelayTime::create(std::max(0.0f, m_delay)));
                const std::string choice = active[i];
                actions->addObject(CallFuncExt::create([activate, choice, player] {
                    auto* layer = PlayLayer::get();
                    if (layer && (layer->m_player1 == player || layer->m_player2 == player)) {
                        activate(choice, player);
                    }
                }));
            }
            if (auto* layer = PlayLayer::get()) layer->runAction(CCSequence::create(actions));
        }
        // Do not call EnhancedGameObject::activatedByPlayer here. CustomRing's
        // vanilla implementation is the reverse-orb action; this object has
        // already dispatched all of its configured orb activations above.
    }

    static PopupOptions getEditSpecialConfig(const Selected& selected) {
        std::vector<std::unique_ptr<ValueMenu>> menus;

        for (size_t slot = 0; slot < 7; ++slot) {
            menus.push_back(EnumMenu::builder()
                .id(fmt::format("orb-{}", slot + 1))
                .title(fmt::format("Orb {}", slot + 1))
                .values(orbChoices())
                .onValue([slot](const std::string& value, const Selected& objects, Popup*) {
                    for (auto* object : objects) if (auto* orb = dynamic_cast<CombinationOrb*>(object)) {
                        auto slots = decodeSlots(orb->m_orbs);
                        slots[slot] = value;
                        orb->m_orbs = encodeSlots(slots);
                    }
                })
                .currentValue([slot](const Selected& objects, Popup*) {
                    for (auto* object : objects) if (auto* orb = dynamic_cast<CombinationOrb*>(object)) {
                        return choiceName(orbChoices(), decodeSlots(orb->m_orbs)[slot]);
                    }
                    return std::string("None");
                }).build());
        }
        menus.push_back(NumericMenu::builder()
            .id("delay").title("Delay (seconds)")
            .inputType(NumericMenu::InputType::Arrows).min(0).max(60).precision(3).stepSize(0.05)
            .onValue([](float value, const Selected& objects, Popup*) {
                applyValueToSelected(objects, &CombinationOrb::m_delay, value);
            })
            .currentValue([](const Selected& objects, Popup*) {
                return getCommonValueOrDefault(objects, &CombinationOrb::m_delay);
            }).build());

        return PopupConfig::builder()
            .width(390).height(285).gapY(8).title("Combination Orb")
            .info(InfoPopup::builder().title("Combination Orb")
                .description("Activates up to seven chosen orbs in order. Delay is the time between each orb. Custom Object Collab orbs are stored by stable ID.")
                .build())
            .menus(std::move(menus)).build();
    }

    std::vector<std::string> getObjectDetails() override {
        std::vector<std::string> details {fmt::format("Delay: {:.3f}s", m_delay)};
        auto choices = orbChoices();
        for (const auto& slot : decodeSlots(m_orbs)) if (!slot.empty()) details.push_back(choiceName(choices, slot));
        return details;
    }
};

// ---------------------------------------------------------------------------
// Gamemode Orb
// ---------------------------------------------------------------------------

class $object(GamemodeOrb, EffectGameObject) {
public:
    static constexpr size_t KEY_GAMEMODE = 155;
    static constexpr size_t KEY_FREE_CAMERA = 156;
    std::string m_gamemode = "v:12";
    bool m_freeCamera = false;

    static GamemodeOrb* create(ObjectInfo* info) { return new GamemodeOrb(info); }
    GamemodeOrb(ObjectInfo* info)
        : CustomObject(info, ObjectTraits::builder().gameObjectType(GameObjectType::CustomRing).build()) {}

    static bool supportsFreeCamera(int portalID) {
        return portalID == 13 || portalID == 111 || portalID == 660 || portalID == 1933;
    }

    static void disableCurrentMode(PlayerObject* player) {
        switch (player->getActiveMode()) {
            case GameObjectType::ShipPortal:   player->toggleFlyMode(false, false); break;
            case GameObjectType::BallPortal:   player->toggleRollMode(false, false); break;
            case GameObjectType::UfoPortal:    player->toggleBirdMode(false, false); break;
            case GameObjectType::WavePortal:   player->toggleDartMode(false, false); break;
            case GameObjectType::RobotPortal:  player->toggleRobotMode(false, false); break;
            case GameObjectType::SpiderPortal: player->toggleSpiderMode(false, false); break;
            case GameObjectType::SwingPortal:  player->toggleSwingMode(false, false); break;
            default: break;
        }
    }

    void activatedByPlayer(PlayerObject* player) override {
        if (m_gamemode.starts_with("c:")) {
            if (auto numericID = resolveObjectChoice(m_gamemode)) {
                if (auto* proxy = GameObject::createWithKey(*numericID)) {
                    proxy->setPosition(this->getPosition());
                    if (auto* portal = dynamic_cast<EffectGameObject*>(proxy)) {
                        portal->m_cameraIsFreeMode = m_freeCamera;
                    }
                    if (auto* custom = dynamic_cast<CustomObjectInterface*>(proxy)) {
                        custom->collidedByPlayer(player);
                    }
                }
            }
        } else if (auto numericID = resolveObjectChoice(m_gamemode)) {
            auto* layer = GJBaseGameLayer::get();
            this->m_cameraIsFreeMode = supportsFreeCamera(*numericID) && m_freeCamera;
            if (layer && *numericID != 12) {
                layer->playerWillSwitchMode(player, this);
                switch (*numericID) {
                    case 13:   layer->switchToFlyMode(player, this, false, static_cast<int>(GameObjectType::ShipPortal)); break;
                    case 47:   layer->switchToRollMode(player, this, false); break;
                    case 111:  layer->switchToFlyMode(player, this, false, static_cast<int>(GameObjectType::UfoPortal)); break;
                    case 660:  layer->switchToFlyMode(player, this, false, static_cast<int>(GameObjectType::WavePortal)); break;
                    case 745:  layer->switchToRobotMode(player, this, false); break;
                    case 1331: layer->switchToSpiderMode(player, this, false); break;
                    case 1933: layer->switchToFlyMode(player, this, false, static_cast<int>(GameObjectType::SwingPortal)); break;
                    default: break;
                }
            } else {
                disableCurrentMode(player);
            }
        }
        // As with Combination Orb, falling through to CustomRing's inherited
        // activation would additionally perform the reverse-orb action.
    }

    static PopupOptions getEditSpecialConfig(const Selected& selected) {
        return PopupConfig::builder().width(360).height(180).title("Gamemode Orb")
            .info(InfoPopup::builder().title("Gamemode Orb")
                .description("Switches the activating player to a vanilla gamemode or a gamemode portal registered by another Object Collab mod.")
                .build())
            .menu(EnumMenu::builder().id("gamemode").title("Gamemode")
                .values(gamemodeChoices())
                .onValue([](const std::string& value, const Selected& objects, Popup*) {
                    applyValueToSelected(objects, &GamemodeOrb::m_gamemode, value);
                })
                .currentValue([](const Selected& objects, Popup*) {
                    const auto value = getCommonValueOrDefault(objects, &GamemodeOrb::m_gamemode);
                    return choiceName(gamemodeChoices(), value);
                }).build())
            .menu(ToggleMenu::builder().id("free-camera").title("Free Camera")
                .onValue([](bool value, const Selected& objects, Popup*) {
                    applyValueToSelected(objects, &GamemodeOrb::m_freeCamera, value);
                })
                .currentValue([](const Selected& objects, Popup*) {
                    return getCommonValueOrDefault(objects, &GamemodeOrb::m_freeCamera);
                }).build()).build();
    }

    std::vector<std::string> getObjectDetails() override {
        return {
            fmt::format("Gamemode: {}", choiceName(gamemodeChoices(), m_gamemode)),
            fmt::format("Free Camera: {}", m_freeCamera ? "Yes" : "No")
        };
    }
};

// ── Registration ──────────────────────────────────────────────────────

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
                PropertyInterface::from(AdvancedOptionsTrigger::KEY_OPTIONS, &AdvancedOptionsTrigger::m_options, ""),
            })
            .build())
        .editObject(AdvancedOptionsTrigger::getEditObjectConfig)
        .build());

    ObjectAPI::registerObject(ObjectInfo::builder()
        .id("combination-orb"_spr).sprite("combination-orb.png"_spr)
        .editorTab(EditorTab::PlayerModifiers).editorButtonColor(EditorButtonColor::Aqua)
        .construction(ComplexObject::builder().factory(CombinationOrb::create)
            .customProperties({
                PropertyInterface::from(CombinationOrb::KEY_ORBS, &CombinationOrb::m_orbs, ""),
                PropertyInterface::from(CombinationOrb::KEY_DELAY, &CombinationOrb::m_delay, 0.0f),
            }).build())
        .editSpecial(CombinationOrb::getEditSpecialConfig).build());

    ObjectAPI::registerObject(ObjectInfo::builder()
        .id("gamemode-orb"_spr).sprite("gamemode-orb.png"_spr)
        .editorTab(EditorTab::PlayerModifiers).editorButtonColor(EditorButtonColor::Pink)
        .construction(ComplexObject::builder().factory(GamemodeOrb::create)
            .customProperties({
                PropertyInterface::from(GamemodeOrb::KEY_GAMEMODE, &GamemodeOrb::m_gamemode, "v:12"),
                PropertyInterface::from(GamemodeOrb::KEY_FREE_CAMERA, &GamemodeOrb::m_freeCamera, false),
            }).build())
        .editSpecial(GamemodeOrb::getEditSpecialConfig).build());
}
