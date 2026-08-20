// Cynth's Objects — a pack of custom objects for Geometry Dash (Geode).
//
// Built on top of Object Collab (smjs.object-collab), which provides the
// custom-object framework and cross-mod compatibility layer.
//
// The "Advanced Options Trigger" lets level makers change any level or
// player setting on the fly.  Each trigger instance changes ONE setting
// to a specific value when activated.  172 options across 6 categories.
// Edit Special now features a searchable Option Browser popup instead of
// a numeric slider for choosing which setting to change.

#include <algorithm>
#include <cstdio>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/ScrollLayer.hpp>
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

static constexpr size_t kOptionCount = sizeof(g_options) / sizeof(g_options[0]);

// ---------------------------------------------------------------------------
// Option Browser Popup — searchable list of all options
// ---------------------------------------------------------------------------

class OptionBrowserPopup : public FLAlertLayer {
protected:
    std::function<void(int)> m_onSelected;
    TextInput*              m_searchInput;
    ScrollLayer*            m_scrollLayer;
    CCMenu*                 m_listMenu;
    std::vector<OptionEntry> m_filtered;
    float                   m_btnHeight;

    bool init(std::function<void(int)> onSelected) {
        m_onSelected = std::move(onSelected);
        m_searchInput = nullptr;
        m_scrollLayer = nullptr;
        m_listMenu = nullptr;
        m_btnHeight = 24.0f;

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        float w = 360.0f;
        float h = 280.0f;

        if (!FLAlertLayer::init(nullptr, "Choose Option", "", "OK", nullptr, w, false, h, 1.0f)) {
            return false;
        }

        auto* bg = this->getChildByType<CCScale9Sprite>(0);
        if (bg) bg->setContentSize({w, h});

        // ── Search bar ────────────────────────────────────────────────
        m_searchInput = TextInput::create(320.0f, "Search options...", "chatFont.fnt");
        m_searchInput->setPosition({w / 2, h - 35.0f});
        m_searchInput->setScale(0.6f);
        m_searchInput->setCallback([this](const std::string& text) {
            this->rebuildList(text);
        });
        this->addChild(m_searchInput);

        // ── Scroll area ───────────────────────────────────────────────
        CCSize listSize = {w - 20.0f, h - 90.0f};
        m_scrollLayer = ScrollLayer::create({10.0f, 10.0f, listSize.width, listSize.height});
        m_scrollLayer->setPosition({0, 0});
        this->addChild(m_scrollLayer);

        // ── Build initial list ────────────────────────────────────────
        rebuildList("");

        // ── OK button hidden (we select by clicking an option) ────────
        // The FLAlertLayer OK button closes the popup; we want selection
        // to close it.  We'll keep the OK button but make it close.
        return true;
    }

    void rebuildList(const std::string& query) {
        // Remove old menu
        if (m_listMenu) {
            m_listMenu->removeFromParent();
            m_listMenu = nullptr;
        }

        // Filter
        m_filtered.clear();
        std::string q = query;
        std::transform(q.begin(), q.end(), q.begin(), ::tolower);

        for (size_t i = 0; i < kOptionCount; ++i) {
            if (q.empty()) {
                m_filtered.push_back(g_options[i]);
            } else {
                std::string name(g_options[i].name);
                std::transform(name.begin(), name.end(), name.begin(), ::tolower);
                std::string cat(g_options[i].category);
                std::transform(cat.begin(), cat.end(), cat.begin(), ::tolower);
                std::string keyStr = std::to_string(g_options[i].key);
                if (name.find(q) != std::string::npos ||
                    cat.find(q) != std::string::npos ||
                    keyStr.find(q) != std::string::npos) {
                    m_filtered.push_back(g_options[i]);
                }
            }
        }

        // Build menu items
        float totalH = m_filtered.size() * m_btnHeight;
        CCSize listSize = m_scrollLayer->getContentSize();
        float contentW = listSize.width;

        m_listMenu = CCMenu::create();
        m_listMenu->setPosition({0, totalH});

        for (size_t i = 0; i < m_filtered.size(); ++i) {
            auto& entry = m_filtered[i];
            float y = totalH - (i + 0.5f) * m_btnHeight;

            // Build label: "[*] #123 - Category: Name"
            char buf[256];
            std::snprintf(buf, sizeof(buf), "[%s] #%d - %s: %s",
                entry.instant ? "*" : "!",
                entry.key,
                entry.category,
                entry.name);

            auto* label = CCLabelBMFont::create(buf, "chatFont.fnt");
            label->setScale(0.35f);
            label->setAnchorPoint({0.0f, 0.5f});

            auto* btn = CCMenuItemSpriteExtra::create(
                label, nullptr, this, menu_selector(OptionBrowserPopup::onOptionSelected));
            btn->setTag(static_cast<int>(i));
            btn->setContentSize({contentW, m_btnHeight});
            btn->setPosition({contentW / 2, y});
            m_listMenu->addChild(btn);
        }

        m_scrollLayer->m_contentLayer->removeAllChildren();
        m_listMenu->setContentSize({contentW, totalH});
        m_scrollLayer->m_contentLayer->addChild(m_listMenu);
        m_scrollLayer->setContentLayerSize({contentW, std::max(totalH, listSize.height)});
    }

    void onOptionSelected(CCObject* sender) {
        int idx = sender->getTag();
        if (idx >= 0 && idx < static_cast<int>(m_filtered.size())) {
            if (m_onSelected) m_onSelected(m_filtered[idx].key);
        }
        this->keyBackClicked();
    }

    void keyBackClicked() override {
        FLAlertLayer::keyBackClicked();
    }

public:
    static OptionBrowserPopup* create(std::function<void(int)> onSelected) {
        auto* p = new OptionBrowserPopup();
        if (p && p->init(std::move(onSelected))) {
            p->autorelease();
            return p;
        }
        delete p;
        return nullptr;
    }
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

// Tiny helper: target for the "Browse..." button's CCMenuItem callback.
// We define it before the trigger class so menu_selector works, but
// implement onTap() after the full trigger definition is visible.
struct PopupButtonHelper final : CCObject {
    void onTap(CCObject* sender);
    static PopupButtonHelper* shared() {
        static PopupButtonHelper* inst = new PopupButtonHelper();
        return inst; // lives for the process lifetime
    }
};

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

// ---------------------------------------------------------------------------
// The trigger object
// ---------------------------------------------------------------------------

class $object(AdvancedOptionsTrigger, EffectGameObject) {
public:
    static constexpr size_t KEY_OPTION = 150;
    static constexpr size_t KEY_VALUE  = 151;

    float m_option = 0.0f;
    float m_value  = 1.0f;

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

    void setOptionKey(int key) {
        m_option = static_cast<float>(key);
    }

    // ── Apply the option ──────────────────────────────────────────────

    void applyOption() {
        auto* pl = PlayLayer::get();
        if (!pl) { log::warn("[AdvOpt] No PlayLayer"); return; }

        auto* settings = pl->m_levelSettings;
        AdvancedOption opt = static_cast<AdvancedOption>(static_cast<int>(m_option));
        float raw = mapValueToFloat(opt, m_value);
        bool  b   = (raw > 0.5f);
        int   i   = static_cast<int>(raw);

        log::info("[AdvOpt] {} = {}", optionName(opt), formatValue(opt, m_value));

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

    // ── Edit Special ──────────────────────────────────────────────────

    static PopupOptions getEditSpecialConfig(const Selected& selected) {
        return PopupConfig::builder()
            .width(380)
            .height(180)
            .title("Advanced Options")
            .info(InfoPopup::builder()
                .title("Advanced Options Trigger")
                .description(
                    "Changes a level or player setting on the fly.\n\n"
                    "Click \"Browse Options\" to search and pick a "
                    "setting from the full list of 172 options.\n\n"
                    "VALUE slider: controls the setting value.\n"
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
    // ── "Browse Options" button ───────────────────────────────────────

    static std::unique_ptr<CustomValueMenu> browseButton() {
        return CustomValueMenu::builder()
            .id("browse")
            .title("Option")
            .factory([](const Selected& selected, geode::Popup* popup) -> CCMenu* {
                auto* menu = CCMenu::create();

                AdvancedOptionsTrigger* trigger = nullptr;
                for (auto* obj : selected) {
                    if (auto* t = dynamic_cast<AdvancedOptionsTrigger*>(obj)) {
                        trigger = t;
                        break;
                    }
                }

                auto* label = CCLabelBMFont::create("Browse Options...", "bigFont.fnt");
                label->setScale(0.5f);

                auto* btn = CCMenuItemSpriteExtra::create(
                    label,
                    nullptr,
                    PopupButtonHelper::shared(),
                    menu_selector(PopupButtonHelper::onTap));
                btn->setUserObject(trigger);
                btn->setContentSize({300.0f, 30.0f});

                menu->addChild(btn);
                menu->setContentSize({300.0f, 30.0f});
                return menu;
            })
            .build();
    }

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

// ── PopupButtonHelper::onTap (implemented after the trigger class) ────

void PopupButtonHelper::onTap(CCObject* sender) {
    auto* btn = static_cast<CCMenuItemSpriteExtra*>(sender);
    auto* trigger = static_cast<AdvancedOptionsTrigger*>(btn->getUserObject());
    if (!trigger) return;

    auto* browser = OptionBrowserPopup::create([trigger](int key) {
        trigger->setOptionKey(key);
    });
    if (browser) browser->show();
}

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
            })
            .build())
        .editSpecial(AdvancedOptionsTrigger::getEditSpecialConfig)
        .build());
}