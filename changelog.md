# Changelog

## v0.5.2

- Fixed the **Browse Options...** button so it opens reliably.
- Fixed the option list rendering blank when the browser opens.
- Added value editing and enable/disable controls to the Option Browser.
  - A single Advanced Options Trigger can now change multiple settings at once.
  - Selecting another row keeps the browser open, making multi-setting setup fast.
  - Existing v0.5.1 single-setting triggers are migrated when first edited and
    remain compatible at runtime.
- Removed the redundant **Value** slider from the main trigger menu.
- Moved the trigger configuration from **Edit Special** to **Edit Object**,
  matching the behavior of most vanilla Geometry Dash triggers.

## v0.5.1

- Replaced the numeric Option Key slider with a **searchable Option Browser**
  popup in Edit Special.
  - Search bar filters by name, category, or key number.
  - Scrollable list of all 172 options.
  - Each row shows `[*]` (instant) or `[!]` (needs respawn/restart).
  - Clicking an option selects it and closes the browser.
- Added a proper trigger-style icon (dark green gear, 64×64 PNG).
- Removed the unused VHS sprite.
- Fixed several compile errors against the real Object Collab / Geode APIs
  (FLAlertLayer init signature, ScrollLayer content layer, NumericMenu input
  types, custom button via `CustomValueMenu`).

## v0.5.0

- Expanded the Advanced Options Trigger from 41 to **172** options across 6
  categories:
  - **Level Settings** (0–15): platformer, mirror, rotate, two-player, reverse,
    start mini/dual, start speed/mode, start pos, no time penalty, reset camera,
    spawn group, target order/channel.
  - **Gameplay** (30–56): practice, ignore damage, freeze player, audio, glitter,
    particles, song trigger interval, section factors, progress height, etc.
  - **Camera** (60–69): shake, offsets, zoom, height.
  - **Physics** (80–91): all bugfixes.
  - **Visual / Debug** (100–115): low detail, indicators, visualizer, hitbox,
    debug draw, background/ground/font/middleground/color indices.
  - **Player** (130–220): effects, particles, streak, animations, glow, controls,
    movement, physics flags, slope handling, collision state, trajectory, ground
    state, velocity.
- Added value types: Bool, Speed, Mode, Int (0–100), Int999 (0–999), Float.
- All player options apply to player 1.

## v0.4.0

- Replaced the VHS shader object with the **Advanced Options Trigger**.
- The trigger changes one level setting to a specific value when activated,
  configured through Edit Special (41 settings across gameplay, camera, physics,
  and player state).

## v0.3.3

- Bugfix: fixed `CCRenderTexture` positioning and fragment shader opacity.

## v0.3.2

- Bugfix: fixed a GLSL `CC_MVPMatrix` redeclaration and added shader logs.

## v0.3.1

- Bugfix: added null-shader guards and a `CCSprite` diagnostic hook.

## v0.3.0

- Rewrote the VHS shader as true post-processing using `CCRenderTexture`.

## v0.2.3

- Bugfix: lowered the Object Collab minimum version to `>=v0.5.5-beta.2` so it
  resolves for everyone on the index (including users still on beta.2). Any
  newer version (including beta.3) remains supported.

## v0.2.2

- Bugfix: corrected the Object Collab dependency. Geode has no true "any
  version" dependency — the index rejects the `*` wildcard during builds
  (HTTP 400 "Bad compare string *") — so the mod depends on a
  `>=v0.5.5-beta.3` minimum bound, which targets the latest release and
  accepts all future updates.

## v0.2.1

- Bugfix: confirmed the Object Collab dependency resolves to the latest
  release (`v0.5.5-beta.3`). Geode dependencies use `>=` / `=` / `<=` version
  ranges (there is no `latest` keyword), so `>=v0.5.5-beta.3` correctly targets
  the newest Object Collab.

## v0.2.0

- Reworked the VHS shader into a full-screen **shader trigger**, modeled after
  vanilla GD shader triggers.
- The effect now **animates** (tracking bands, grain) over time.
- Added adjustable **Fade In**, **Fade Out**, **Hold** and **Opacity** timing.
- Each part of the effect (**Scanlines**, **Grain**, **Chromatic**, **Tracking**,
  **Vignette**, **Speed**) is now independently adjustable from **Edit Special**.
- Touch / spawn / multi trigger support, and the effect can be stopped early
  with a Stop trigger.

## v0.1.0

- Initial release.
- Added the **VHS Shader** object (a full-screen decoration overlay).
