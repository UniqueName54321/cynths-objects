// Cynth's Objects — a pack of custom objects for Geometry Dash (Geode).
//
// Built on top of Object Collab (smjs.object-collab), which provides the
// custom-object framework and cross-mod compatibility layer.
//
// The "Advanced Options Trigger" lets level makers change any level or
// player setting on the fly.  Each trigger instance changes ONE setting
// to a specific value when activated.  150+ options across 6 categories.

#include <algorithm>

#include <Geode/Geode.hpp>
#include <smjs.object-collab/include/object_collab.hpp>

using namespace geode::prelude;
using namespace object_collab::prelude;

// ---------------------------------------------------------------------------
// Advanced Options Trigger — option definitions
// ---------------------------------------------------------------------------
//
// Bool:   value <= 0.5 → false,  value > 0.5 → true
// Speed:  value 0→1 maps to 0=Slow, 0.25=Normal, 0.5=Fast,
//         0.75=Very Fast, 1=Extreme
// Mode:   value 0→1 maps to 0=Cube, 0.14=Ship, 0.29=Ball, 0.43=UFO,
//         0.57=Wave, 0.71=Robot, 0.86=Spider, 1=Swing
// Int:    value 0→1 maps to an integer range (documented per option)
//
// ★ = instant   ⚡ = needs respawn/restart

enum class AdvancedOption : int {
    // ── Level Settings (0–29) — mostly from LevelSettingsObject ───────
    PlatformerMode      =  0,  // ★  bool   platformer mode
    MirrorMode          =  1,  // ⚡  bool   mirror gameplay
    RotateGameplay      =  2,  // ⚡  bool   rotate gameplay 90°
    TwoPlayerMode       =  3,  // ⚡  bool   two-player mode
    ReverseGameplay     =  4,  // ⚡  bool   reverse level direction
    StartMini           =  5,  // ⚡  bool   start as mini icon
    StartDual           =  6,  // ⚡  bool   start as dual
    StartSpeed          =  7,  // ⚡  speed  starting speed
    StartMode           =  8,  // ⚡  mode   starting gamemode
    StartsWithStartPos  =  9,  // ⚡  bool   start from start pos
    DisableStartPos     = 10,  // ⚡  bool   disable start positions
    NoTimePenalty       = 11,  // ★  bool   no time penalty
    ResetCamera         = 12,  // ★  bool   reset camera on trigger
    SpawnGroup          = 13,  // ⚡  int    spawn group (0–999)
    TargetOrder         = 14,  // ⚡  int    target order (0–999)
    TargetChannel       = 15,  // ⚡  int    target channel (0–999)

    // ── PlayLayer / Gameplay (30–59) ─────────────────────────────────
    PracticeMode        = 30,  // ★  bool   practice mode
    PracticeMusicSync   = 31,  // ★  bool   sync music in practice
    IgnoreDamage        = 32,  // ★  bool   noclip / ignore damage
    DualTouchTrigger    = 33,  // ★  bool   dual-touch trigger mode
    FreezePlayer        = 34,  // ★  bool   freeze player controls
    AudioPaused         = 35,  // ★  bool   pause audio
    StartOptimization   = 36,  // ★  bool   start optimization
    SkipAudioStep       = 37,  // ★  bool   skip audio processing
    LoadingStartPos     = 38,  // ★  bool   loading start position
    ProcessingAudioTrig = 39,  // ★  bool   processing audio triggers
    KeepGroupParents    = 40,  // ★  bool   keep group parents
    ResetActiveObjects  = 41,  // ★  bool   reset active objects
    SkipArtReload       = 42,  // ★  bool   skip art reload
    UpdateGroundShadows = 43,  // ★  bool   update ground shadows
    RecordInputs        = 44,  // ★  bool   record player inputs
    DisableGravityEffect= 45,  // ★  bool   disable gravity effect
    GlitterEnabled      = 46,  // ★  bool   enable glitter effect
    BGEffectDisabled    = 47,  // ★  bool   disable BG effect
    DecimalPercentage   = 48,  // ★  bool   show decimal percentage
    ShowLeaderboardPct  = 49,  // ★  bool   show leaderboard %
    SongTriggerInterval = 50,  // ★  float  song trigger interval (0–10)
    ProgressHeight      = 51,  // ★  float  progress bar height (0–200)
    SectionXFactor      = 52,  // ★  float  section X factor (0–2)
    SectionYFactor      = 53,  // ★  float  section Y factor (0–2)
    ParticleCount       = 54,  // ★  int    particle count (0–500)
    CustomParticleCount = 55,  // ★  int    custom particle count (0–500)
    ParticleSystemLimit = 56,  // ★  int    particle system limit (0–500)

    // ── Camera (60–79) ───────────────────────────────────────────────
    StaticCameraShake   = 60,  // ★  bool   static camera (no shake)
    SkipCameraShake     = 61,  // ★  bool   skip all camera shake
    FreezeStartCamera   = 62,  // ★  bool   freeze camera at start
    CameraWidthOffset   = 63,  // ★  float  camera width offset (-500–500)
    CameraHeightOffset  = 64,  // ★  float  camera height offset (-500–500)
    CameraUnzoomedX     = 65,  // ★  float  unzoomed camera X (0–2000)
    HalfCameraWidth     = 66,  // ★  float  half camera width (0–1000)
    UnzoomedHeightOffset= 67,  // ★  float  unzoomed height offset (-500–500)
    TargetHeightOffset  = 68,  // ★  float  target camera height (-500–500)
    CalcTargetHeight    = 69,  // ★  bool   calculate target height offset

    // ── Physics / Bugfixes (80–99) ───────────────────────────────────
    FixGravityBug       = 80,  // ★  bool   fix gravity bug
    FixNegativeScale    = 81,  // ★  bool   fix negative scale
    FixRobotJump        = 82,  // ★  bool   fix robot jump
    FixRadiusCollision  = 83,  // ★  bool   fix radius collision
    DynamicLevelHeight  = 84,  // ★  bool   dynamic level height
    AllowMultiRotation  = 85,  // ★  bool   allow multi-rotation
    EnablePlayerSqueeze = 86,  // ★  bool   enable player squeeze
    Enable22Changes     = 87,  // ★  bool   enable 2.2 changes
    AllowStaticRotate   = 88,  // ★  bool   allow static rotate
    ReverseSync         = 89,  // ★  bool   reverse sync
    DecreaseBoostSlide  = 90,  // ★  bool   decrease boost slide
    EnableImpulseFix    = 91,  // ★  bool   enable impulse fix

    // ── Visual / Debug (100–129) ─────────────────────────────────────
    LowDetailMode       = 100, // ★  bool   low detail mode
    PortalIndicators    = 101, // ★  bool   show portal indicators
    OrbIndicators       = 102, // ★  bool   show orb indicators
    ShowAudioVisualizer = 103, // ★  bool   show audio visualizer
    DisablePlayerHitbox = 104, // ★  bool   hide player hitbox
    HitboxesOnDeath     = 105, // ★  bool   show hitboxes on death
    DebugDraw           = 106, // ★  bool   enable debug draw
    LevelEndAnimStarted = 107, // ★  bool   level end animation started
    ObjectsDeactivated  = 108, // ★  bool   deactivate all objects
    ActiveGradients     = 109, // ★  int    active gradients (0–100)
    BackgroundIndex     = 110, // ⚡  int    background index (0–99)
    GroundIndex         = 111, // ⚡  int    ground index (0–99)
    FontIndex           = 112, // ⚡  int    font index (0–99)
    MiddleGroundIndex   = 113, // ⚡  int    middle ground index (0–99)
    ColorPage           = 114, // ⚡  int    color page (0–9)
    GroundLineIndex     = 115, // ⚡  int    ground line index (0–99)

    // ── Player Options (130–169) ─────────────────────────────────────
    PlayerPlayEffects   = 130, // ★  bool   enable player effects
    PlayerGroundParticles=131, // ★  bool   enable ground particles
    PlayerShipParticles  =132, // ★  bool   enable ship particles
    PlayerRobotAnim1    = 133, // ★  bool   enable robot animation 1
    PlayerRobotAnim2    = 134, // ★  bool   enable robot animation 2
    PlayerSpiderAnim    = 135, // ★  bool   enable spider animation
    PlayerStreakType    = 136, // ★  int    player streak type (0–10)
    PlayerStreakWidth   = 137, // ★  float  streak stroke width (0–10)
    PlayerDisableStreakTint=138,//★  bool   disable streak tint
    PlayerAlwaysShowStreak=139,// ★  bool   always show streak
    PlayerFadeOutStreak = 140, // ★  bool   fade out streak
    PlayerSwitchWaveColor=141, // ★  bool   switch wave trail color
    PlayerSwitchDashFire =142, // ★  bool   switch dash fire color
    PlayerHasCustomGlow = 143, // ★  bool   custom glow color
    PlayerVehicleGlowing= 144, // ★  bool   vehicle glowing
    PlayerDefaultMiniIcon=145, // ★  bool   default mini icon
    PlayerControlsDisabled=146,// ★  bool   disable player controls
    PlayerInputsLocked  = 147, // ★  bool   lock player inputs
    PlayerPracticeDeath = 148, // ★  bool   practice death effect
    PlayerReducedEffects= 149, // ★  bool   reduced effects
    PlayerQuickCheckpoint=150, // ★  bool   quick checkpoint mode
    PlayerCanPlaceCheckpoint=151,//★  bool   allow placing checkpoint
    PlayerAffectedByForces=152,// ★  bool   affected by forces
    PlayerTrailParticleLife=153,//★  float  trail particle life (0–5)
    PlayerLandParticlesAngle=154,//★  float  land particles angle (0–360)
    PlayerLandParticleY  =155, // ★  float  land particle Y offset (-50–50)
    PlayerUseLandParticles=156,// ★  bool   use land particles
    PlayerIsPlatformer  = 157, // ★  bool   player is platformer
    PlayerIgnoreDamage  = 158, // ★  bool   player ignores damage
    PlayerFixGravityBug = 159, // ★  bool   player fix gravity bug
    PlayerFixRobotJump  = 160, // ★  bool   player fix robot jump
    PlayerReverseSync   = 161, // ★  bool   player reverse sync
    PlayerDecreaseBoost = 162, // ★  bool   player decrease boost slide
    PlayerEnable22      = 163, // ★  bool   player enable 2.2 changes
    PlayerEnableImpulseFix=164,//★  bool   player enable impulse fix
    PlayerDisableSqueeze= 165, // ★  bool   player disable squeeze
    PlayerIsOutOfBounds = 166, // ★  bool   player is out of bounds
    PlayerIsSecondPlayer= 167, // ★  bool   player is second player
    PlayerIsUpsideDown  = 168, // ★  bool   player is upside down
    PlayerIsGoingLeft   = 169, // ★  bool   player is going left
    PlayerHoldingRight  = 170, // ★  bool   hold right
    PlayerHoldingLeft   = 171, // ★  bool   hold left
    PlayerPlatformerMovingLeft =172,//★  bool   platformer moving left
    PlayerPlatformerMovingRight=173,//★  bool   platformer moving right
    PlayerPlatformerXVelocity=174,//★  float  platformer X velocity (-500–500)
    PlayerAcceleration  = 175, // ★  float  acceleration/speed (0–500)
    PlayerXTrajectory   = 176, // ★  float  X velocity related (-500–500)
    PlayerYTrajectory   = 177, // ★  float  Y velocity related (-500–500)
    PlayerScaleXTime    = 178, // ★  float  scale X related time (0–10)
    PlayerStateOnGround = 179, // ★  int    on-ground state (0–5)
    PlayerStateBoostX   = 180, // ★  int    boost X state (0–5)
    PlayerStateBoostY   = 181, // ★  int    boost Y state (0–5)
    PlayerStateScale    = 182, // ★  int    scale state (0–5)
    PlayerStateForce    = 183, // ★  int    force state (0–5)
    PlayerStateFlipGrav = 184, // ★  int    flip gravity state (0–5)
    PlayerStateNoAutoJump=185, // ★  int    no-auto-jump state (0–5)
    PlayerStateDartSlide= 186, // ★  int    dart slide state (0–5)
    PlayerStateHitHead  = 187, // ★  int    hit head state (0–5)
    PlayerIsAccelerating= 188, // ★  bool   player is accelerating
    PlayerIsBoosted     = 189, // ★  bool   player is boosted
    PlayerHasEverJumped = 190, // ★  bool   player has ever jumped
    PlayerHasEverHitRing= 191, // ★  bool   player has ever hit ring
    PlayerJumpBuffered  = 192, // ★  bool   jump buffered
    PlayerWasJumpBuffered=193, // ★  bool   was jump buffered
    PlayerStateRingJump = 194, // ★  bool   ring jump state
    PlayerTouchedRing   = 195, // ★  bool   touched ring
    PlayerTouchedCustomRing=196,//★  bool   touched custom ring
    PlayerTouchedGravityPortal=197,//★ bool   touched gravity portal
    PlayerIsColliding   = 198, // ★  bool   player is colliding
    PlayerIsOnGround    = 199, // ★  bool   player is on ground
    PlayerIsFalling     = 200, // ★  bool   player is falling
    PlayerIsSlidingRight= 201, // ★  bool   player is sliding right
    PlayerIsSlopeTop    = 202, // ★  bool   player is on slope top
    PlayerSlopeFlipGrav = 203, // ★  bool   slope flip gravity
    PlayerSlopeAngle    = 204, // ★  float  slope angle (radians, 0–3.14)
    PlayerSlopeSliding  = 205, // ★  bool   slope sliding (maybe rotated)
    PlayerCollidingSlope= 206, // ★  bool   is colliding with slope
    PlayerBallRotating  = 207, // ★  bool   ball is rotating
    PlayerGroundMaterial =208, // ★  int    ground object material (0–99)
    PlayerDashFireFrame = 209, // ★  int    dash fire frame (0–100)
    PlayerFollowRelated = 210, // ★  int    follow related (0–100)
    PlayerReverseRelated= 211, // ★  int    reverse related (0–100)
    PlayerIconRequestID = 212, // ★  int    icon request ID (0–999)
    PlayerWasTeleported = 213, // ★  bool   player was teleported
    PlayerJustPlacedStreak=214,// ★  bool   just placed streak
    PlayerPadRingRelated= 215, // ★  bool   pad/ring related
    PlayerCanRunIntoBlocks=216,//★  bool   can run into blocks
    PlayerRingJumpRelated=217, // ★  bool   ring jump related
    PlayerMaybeSpriteRelated=218,//★ bool   sprite related
    PlayerHasStopped    = 219, // ★  bool   player has stopped
    PlayerGoingCorrectSlope=220,//★  bool   going correct slope direction
};

// Human-readable name.
static const char* optionName(AdvancedOption opt) {
    switch (opt) {
        // Level Settings
        case AdvancedOption::PlatformerMode:      return "Platformer Mode";
        case AdvancedOption::MirrorMode:          return "Mirror Mode";
        case AdvancedOption::RotateGameplay:      return "Rotate Gameplay";
        case AdvancedOption::TwoPlayerMode:       return "Two-Player Mode";
        case AdvancedOption::ReverseGameplay:     return "Reverse Gameplay";
        case AdvancedOption::StartMini:           return "Start Mini";
        case AdvancedOption::StartDual:           return "Start Dual";
        case AdvancedOption::StartSpeed:          return "Start Speed";
        case AdvancedOption::StartMode:           return "Start Game Mode";
        case AdvancedOption::StartsWithStartPos:  return "Start w/ Start Pos";
        case AdvancedOption::DisableStartPos:     return "Disable Start Pos";
        case AdvancedOption::NoTimePenalty:       return "No Time Penalty";
        case AdvancedOption::ResetCamera:         return "Reset Camera";
        case AdvancedOption::SpawnGroup:          return "Spawn Group";
        case AdvancedOption::TargetOrder:         return "Target Order";
        case AdvancedOption::TargetChannel:       return "Target Channel";
        // PlayLayer / Gameplay
        case AdvancedOption::PracticeMode:        return "Practice Mode";
        case AdvancedOption::PracticeMusicSync:   return "Practice Music Sync";
        case AdvancedOption::IgnoreDamage:        return "Ignore Damage";
        case AdvancedOption::DualTouchTrigger:    return "Dual-Touch Trigger";
        case AdvancedOption::FreezePlayer:        return "Freeze Player";
        case AdvancedOption::AudioPaused:         return "Audio Paused";
        case AdvancedOption::StartOptimization:   return "Start Optimization";
        case AdvancedOption::SkipAudioStep:       return "Skip Audio Step";
        case AdvancedOption::LoadingStartPos:     return "Loading Start Pos";
        case AdvancedOption::ProcessingAudioTrig: return "Processing Audio Trig";
        case AdvancedOption::KeepGroupParents:    return "Keep Group Parents";
        case AdvancedOption::ResetActiveObjects:  return "Reset Active Objects";
        case AdvancedOption::SkipArtReload:       return "Skip Art Reload";
        case AdvancedOption::UpdateGroundShadows: return "Update Ground Shadows";
        case AdvancedOption::RecordInputs:        return "Record Inputs";
        case AdvancedOption::DisableGravityEffect:return "Disable Gravity Effect";
        case AdvancedOption::GlitterEnabled:      return "Glitter Enabled";
        case AdvancedOption::BGEffectDisabled:    return "BG Effect Disabled";
        case AdvancedOption::DecimalPercentage:   return "Decimal Percentage";
        case AdvancedOption::ShowLeaderboardPct:  return "Show Leaderboard %";
        case AdvancedOption::SongTriggerInterval: return "Song Trigger Interval";
        case AdvancedOption::ProgressHeight:      return "Progress Bar Height";
        case AdvancedOption::SectionXFactor:      return "Section X Factor";
        case AdvancedOption::SectionYFactor:      return "Section Y Factor";
        case AdvancedOption::ParticleCount:       return "Particle Count";
        case AdvancedOption::CustomParticleCount: return "Custom Particle Count";
        case AdvancedOption::ParticleSystemLimit: return "Particle System Limit";
        // Camera
        case AdvancedOption::StaticCameraShake:   return "Static Camera Shake";
        case AdvancedOption::SkipCameraShake:     return "Skip Camera Shake";
        case AdvancedOption::FreezeStartCamera:   return "Freeze Start Camera";
        case AdvancedOption::CameraWidthOffset:   return "Camera Width Offset";
        case AdvancedOption::CameraHeightOffset:  return "Camera Height Offset";
        case AdvancedOption::CameraUnzoomedX:     return "Camera Unzoomed X";
        case AdvancedOption::HalfCameraWidth:     return "Half Camera Width";
        case AdvancedOption::UnzoomedHeightOffset:return "Unzoomed Height Offset";
        case AdvancedOption::TargetHeightOffset:  return "Target Height Offset";
        case AdvancedOption::CalcTargetHeight:    return "Calc Target Height";
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
        case AdvancedOption::LevelEndAnimStarted: return "Level End Anim Started";
        case AdvancedOption::ObjectsDeactivated:  return "Objects Deactivated";
        case AdvancedOption::ActiveGradients:     return "Active Gradients";
        case AdvancedOption::BackgroundIndex:     return "Background Index";
        case AdvancedOption::GroundIndex:         return "Ground Index";
        case AdvancedOption::FontIndex:           return "Font Index";
        case AdvancedOption::MiddleGroundIndex:   return "Middle Ground Index";
        case AdvancedOption::ColorPage:           return "Color Page";
        case AdvancedOption::GroundLineIndex:     return "Ground Line Index";
        // Player
        case AdvancedOption::PlayerPlayEffects:   return "Player: Play Effects";
        case AdvancedOption::PlayerGroundParticles:return "Player: Ground Particles";
        case AdvancedOption::PlayerShipParticles:  return "Player: Ship Particles";
        case AdvancedOption::PlayerRobotAnim1:    return "Player: Robot Anim 1";
        case AdvancedOption::PlayerRobotAnim2:    return "Player: Robot Anim 2";
        case AdvancedOption::PlayerSpiderAnim:    return "Player: Spider Anim";
        case AdvancedOption::PlayerStreakType:    return "Player: Streak Type";
        case AdvancedOption::PlayerStreakWidth:   return "Player: Streak Width";
        case AdvancedOption::PlayerDisableStreakTint:return "Player: Disable Streak Tint";
        case AdvancedOption::PlayerAlwaysShowStreak:return "Player: Always Show Streak";
        case AdvancedOption::PlayerFadeOutStreak: return "Player: Fade Out Streak";
        case AdvancedOption::PlayerSwitchWaveColor:return "Player: Switch Wave Color";
        case AdvancedOption::PlayerSwitchDashFire:return "Player: Switch Dash Fire";
        case AdvancedOption::PlayerHasCustomGlow: return "Player: Custom Glow";
        case AdvancedOption::PlayerVehicleGlowing:return "Player: Vehicle Glowing";
        case AdvancedOption::PlayerDefaultMiniIcon:return "Player: Default Mini Icon";
        case AdvancedOption::PlayerControlsDisabled:return "Player: Controls Disabled";
        case AdvancedOption::PlayerInputsLocked:  return "Player: Inputs Locked";
        case AdvancedOption::PlayerPracticeDeath: return "Player: Practice Death";
        case AdvancedOption::PlayerReducedEffects:return "Player: Reduced Effects";
        case AdvancedOption::PlayerQuickCheckpoint:return "Player: Quick Checkpoint";
        case AdvancedOption::PlayerCanPlaceCheckpoint:return "Player: Can Place Checkpoint";
        case AdvancedOption::PlayerAffectedByForces:return "Player: Affected By Forces";
        case AdvancedOption::PlayerTrailParticleLife:return "Player: Trail Particle Life";
        case AdvancedOption::PlayerLandParticlesAngle:return "Player: Land Particle Angle";
        case AdvancedOption::PlayerLandParticleY: return "Player: Land Particle Y";
        case AdvancedOption::PlayerUseLandParticles:return "Player: Use Land Particles";
        case AdvancedOption::PlayerIsPlatformer:  return "Player: Is Platformer";
        case AdvancedOption::PlayerIgnoreDamage:  return "Player: Ignore Damage";
        case AdvancedOption::PlayerFixGravityBug: return "Player: Fix Gravity Bug";
        case AdvancedOption::PlayerFixRobotJump:  return "Player: Fix Robot Jump";
        case AdvancedOption::PlayerReverseSync:   return "Player: Reverse Sync";
        case AdvancedOption::PlayerDecreaseBoost: return "Player: Decrease Boost";
        case AdvancedOption::PlayerEnable22:      return "Player: Enable 2.2";
        case AdvancedOption::PlayerEnableImpulseFix:return "Player: Impulse Fix";
        case AdvancedOption::PlayerDisableSqueeze:return "Player: Disable Squeeze";
        case AdvancedOption::PlayerIsOutOfBounds: return "Player: Out Of Bounds";
        case AdvancedOption::PlayerIsSecondPlayer:return "Player: Is Second Player";
        case AdvancedOption::PlayerIsUpsideDown:  return "Player: Upside Down";
        case AdvancedOption::PlayerIsGoingLeft:   return "Player: Going Left";
        case AdvancedOption::PlayerHoldingRight:  return "Player: Holding Right";
        case AdvancedOption::PlayerHoldingLeft:   return "Player: Holding Left";
        case AdvancedOption::PlayerPlatformerMovingLeft:return "Player: Plat Moving Left";
        case AdvancedOption::PlayerPlatformerMovingRight:return "Player: Plat Moving Right";
        case AdvancedOption::PlayerPlatformerXVelocity:return "Player: Plat X Velocity";
        case AdvancedOption::PlayerAcceleration:  return "Player: Acceleration";
        case AdvancedOption::PlayerXTrajectory:   return "Player: X Trajectory";
        case AdvancedOption::PlayerYTrajectory:   return "Player: Y Trajectory";
        case AdvancedOption::PlayerScaleXTime:    return "Player: Scale X Time";
        case AdvancedOption::PlayerStateOnGround: return "Player: State On Ground";
        case AdvancedOption::PlayerStateBoostX:   return "Player: State Boost X";
        case AdvancedOption::PlayerStateBoostY:   return "Player: State Boost Y";
        case AdvancedOption::PlayerStateScale:    return "Player: State Scale";
        case AdvancedOption::PlayerStateForce:    return "Player: State Force";
        case AdvancedOption::PlayerStateFlipGrav: return "Player: State Flip Grav";
        case AdvancedOption::PlayerStateNoAutoJump:return "Player: State No Auto Jump";
        case AdvancedOption::PlayerStateDartSlide:return "Player: State Dart Slide";
        case AdvancedOption::PlayerStateHitHead:  return "Player: State Hit Head";
        case AdvancedOption::PlayerIsAccelerating:return "Player: Is Accelerating";
        case AdvancedOption::PlayerIsBoosted:     return "Player: Is Boosted";
        case AdvancedOption::PlayerHasEverJumped: return "Player: Has Ever Jumped";
        case AdvancedOption::PlayerHasEverHitRing:return "Player: Has Ever Hit Ring";
        case AdvancedOption::PlayerJumpBuffered:  return "Player: Jump Buffered";
        case AdvancedOption::PlayerWasJumpBuffered:return "Player: Was Jump Buffered";
        case AdvancedOption::PlayerStateRingJump: return "Player: Ring Jump State";
        case AdvancedOption::PlayerTouchedRing:   return "Player: Touched Ring";
        case AdvancedOption::PlayerTouchedCustomRing:return "Player: Touched Custom Ring";
        case AdvancedOption::PlayerTouchedGravityPortal:return "Player: Touched Grav Portal";
        case AdvancedOption::PlayerIsColliding:   return "Player: Is Colliding";
        case AdvancedOption::PlayerIsOnGround:    return "Player: Is On Ground";
        case AdvancedOption::PlayerIsFalling:     return "Player: Is Falling";
        case AdvancedOption::PlayerIsSlidingRight:return "Player: Sliding Right";
        case AdvancedOption::PlayerIsSlopeTop:    return "Player: Is Slope Top";
        case AdvancedOption::PlayerSlopeFlipGrav: return "Player: Slope Flip Grav";
        case AdvancedOption::PlayerSlopeAngle:    return "Player: Slope Angle";
        case AdvancedOption::PlayerSlopeSliding:  return "Player: Slope Sliding";
        case AdvancedOption::PlayerCollidingSlope:return "Player: Colliding w/ Slope";
        case AdvancedOption::PlayerBallRotating:  return "Player: Ball Rotating";
        case AdvancedOption::PlayerGroundMaterial:return "Player: Ground Material";
        case AdvancedOption::PlayerDashFireFrame: return "Player: Dash Fire Frame";
        case AdvancedOption::PlayerFollowRelated: return "Player: Follow Related";
        case AdvancedOption::PlayerReverseRelated:return "Player: Reverse Related";
        case AdvancedOption::PlayerIconRequestID: return "Player: Icon Request ID";
        case AdvancedOption::PlayerWasTeleported: return "Player: Was Teleported";
        case AdvancedOption::PlayerJustPlacedStreak:return "Player: Just Placed Streak";
        case AdvancedOption::PlayerPadRingRelated:return "Player: Pad/Ring Related";
        case AdvancedOption::PlayerCanRunIntoBlocks:return "Player: Can Run Into Block";
        case AdvancedOption::PlayerRingJumpRelated:return "Player: Ring Jump Related";
        case AdvancedOption::PlayerMaybeSpriteRelated:return "Player: Sprite Related";
        case AdvancedOption::PlayerHasStopped:    return "Player: Has Stopped";
        case AdvancedOption::PlayerGoingCorrectSlope:return "Player: Correct Slope Dir";
    }
    return "???";
}

// ── Value type (determines how 0–1 slider maps to the actual field) ──

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
        // Float types: map to meaningful ranges
        case OptionType::Float:  return slider;
        case OptionType::Bool:   return (slider > 0.5f) ? 1.0f : 0.0f;
    }
    return slider;
}

static std::string formatValue(AdvancedOption opt, float slider) {
    float v = mapValueToFloat(opt, slider);
    OptionType t = optionType(opt);
    switch (t) {
        case OptionType::Bool: return (v > 0.5f) ? "ON" : "OFF";
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

// ── The trigger object ────────────────────────────────────────────────

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

    // ── Helpers ───────────────────────────────────────────────────────

    template <typename F>
    void applyToPlayer1(F&& fn) {
        auto* pl = PlayLayer::get();
        if (!pl) return;
        if (pl->m_player1) fn(pl->m_player1);
    }

    // ── Edit Special ──────────────────────────────────────────────────

    static PopupOptions getEditSpecialConfig(const Selected& selected) {
        static const char* helpText = R"(
Changes a level or player setting on the fly.

OPTION KEY: 0-220 (see categories below)
VALUE: 0-1 slider mapped to the option type.

Bool: 0→0.5=OFF  0.5→1=ON
Speed: 0=Slow .25=Normal .5=Fast .75=V.Fast 1=Extreme
Mode: 0=Cube .14=Ship .29=Ball .43=UFO .57=Wave .71=Robot .86=Spider 1=Swing
Int:   0→1 maps to 0→100 (or 0→999 for spawn/target)
Float: 0→1 maps to the range documented per option

★=instant  ⚡=needs respawn

── LEVEL SETTINGS (0-15) ──
0 ★Platformer    1 ⚡Mirror       2 ⚡Rotate
3 ⚡TwoPlayer    4 ⚡Reverse      5 ⚡StartMini
6 ⚡StartDual    7 ⚡StartSpeed   8 ⚡StartMode
9 ⚡StartPosOn  10 ⚡DisableStart 11 ★NoTimePenalty
12 ★ResetCam   13 ⚡SpawnGroup   14 ⚡TargetOrder
15 ⚡TargetChan

── GAMEPLAY (30-56) ──
30 ★Practice    31 ★PracticeSync 32 ★IgnoreDamage
33 ★DualTouch   34 ★FreezePlayer 35 ★AudioPaused
36 ★StartOpt    37 ★SkipAudio    38 ★LoadingStart
39 ★ProcAudio   40 ★KeepParents  41 ★ResetObjs
42 ★SkipArtRel  43 ★GroundShadows 44 ★RecordInputs
45 ★NoGravityEff 46 ★Glitter     47 ★BGEffectOff
48 ★DecimalPct  49 ★Leaderboard% 50 ★SongTrigInt
51 ★ProgHeight  52 ★SectionX     53 ★SectionY
54 ★ParticleCnt 55 ★CustomPrtCnt 56 ★ParticleLim

── CAMERA (60-69) ──
60 ★StaticShake 61 ★SkipShake    62 ★FreezeCam
63 ★CamWidthOff 64 ★CamHeightOff 65 ★CamUnzoomedX
66 ★HalfCamW    67 ★UnzoomHOff   68 ★TargetHOff
69 ★CalcTargetH

── PHYSICS (80-91) ──
80 ★FixGravity  81 ★FixNegScale  82 ★FixRobotJump
83 ★FixRadius   84 ★DynamicH     85 ★MultiRotation
86 ★Squeeze     87 ★2.2Changes   88 ★StaticRotate
89 ★ReverseSync 90 ★DecBoost     91 ★ImpulseFix

── VISUAL (100-115) ──
100 ★LowDetail  101 ★PortalIndic  102 ★OrbIndic
103 ★AudioVis   104 ★NoHitbox     105 ★DeathHitbox
106 ★DebugDraw  107 ★EndAnim      108 ★ObjsDeact
109 ★Gradients  110 ⚡BgIndex      111 ⚡GroundIndex
112 ⚡FontIndex  113 ⚡MidGrIndex   114 ⚡ColorPage
115 ⚡GroundLine

── PLAYER (130-220) ──
130 ★PlayEff    131 ★GrndPrtcl    132 ★ShipPrtcl
133 ★RobotAnim1 134 ★RobotAnim2   135 ★SpiderAnim
136 ★StreakType 137 ★StreakWidth  138 ★NoStreakTint
139 ★AlwaysStreak 140 ★FadeStreak 141 ★WaveColor
142 ★DashFire   143 ★CustomGlow   144 ★VehicleGlow
145 ★MiniIcon   146 ★CtrlDisabled 147 ★InputLocked
148 ★PracDeath  149 ★ReducedEff   150 ★QuickCheckpt
151 ★CanPlaceCP 152 ★AffectedByForce 153 ★TrailLife
154 ★LandAngle  155 ★LandY        156 ★UseLandPrtcl
157 ★IsPlat     158 ★IgnoreDmg    159 ★FixGrav
160 ★FixRobot   161 ★RevSync      162 ★DecBoost
163 ★Enable22   164 ★ImpulseFix   165 ★NoSqueeze
166 ★OutOfBounds 167 ★2ndPlayer   168 ★UpsideDown
169 ★GoingLeft  170 ★HoldRight    171 ★HoldLeft
172 ★PlatMovL   173 ★PlatMovR     174 ★PlatXVel
175 ★Accel      176 ★XTraj        177 ★YTraj
178 ★ScaleXTime 179 ★StateGround  180 ★StateBoostX
181 ★StateBoostY 182 ★StateScale  183 ★StateForce
184 ★StateFlipG 185 ★NoAutoJump   186 ★DartSlide
187 ★HitHead    188 ★AccelFlag    189 ★Boosted
190 ★EverJumped 191 ★EverHitRing  192 ★JumpBuffered
193 ★WasJumpBuf 194 ★RingJump     195 ★TouchRing
196 ★TouchCustRing 197 ★TouchGrav 198 ★Colliding
199 ★OnGround   200 ★Falling      201 ★SlidingRight
202 ★SlopeTop   203 ★SlopeFlipG   204 ★SlopeAngle
205 ★SlopeSlid  206 ★CollidSlope  207 ★BallRotate
208 ★GrndMat    209 ★DashFireFr   210 ★FollowRel
211 ★RevRel     212 ★IconReqID    213 ★Teleported
214 ★JustStreak 215 ★PadRing      216 ★CanRunBlock
217 ★RingJumpRel 218 ★SpriteRel   219 ★HasStopped
220 ★CorrectSlope
)";
        return PopupConfig::builder()
            .width(400)
            .height(240)
            .title("Advanced Options")
            .info(InfoPopup::builder()
                .title("Advanced Options Trigger")
                .description(helpText)
                .build())
            .triggerToggles(true)
            .menu(slider("option"_spr, "Option Key", 0.f, 220.f, 1.f,
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