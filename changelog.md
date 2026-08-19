# Changelog

## v0.2.2

- Bugfix: removed the Object Collab version pin. The dependency now uses `*`
  (any version), so the mod always runs against whatever Object Collab is
  installed rather than requiring a specific minimum version.

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
