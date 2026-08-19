# Cynth's Objects

A [Geode](https://geode-sdk.org/) mod for Geometry Dash that adds custom
objects to the editor, built on top of
[Object Collab](https://geode-sdk.org/mods/smjs.object-collab).

## Objects

| Object      | Type    | Description |
|-------------|---------|-------------|
| VHS Shader  | Trigger | A full-screen VHS-style shader trigger — scanlines, film grain, chromatic fringing, tracking bands and vignette. Fades in/out and every effect is independently adjustable from its "Edit Special" screen. |

## Usage

1. Place the **VHS Shader** trigger in the level.
2. Select it and open **Edit Special** (the wrench). Use the touch/spawn/multi
   trigger toggles to decide how it activates, then tweak each effect
   (scanlines, grain, chromatic, tracking, vignette, speed) and the timing
   (fade in/out, hold, opacity).
3. Play the level — triggering it fades the VHS effect in over the whole
   screen. It fades out after the **Hold** time (set **Hold** to `0` to keep it
   on until level end), or early if a Stop trigger targets its group.

> Only one VHS overlay is shown at a time, even if you place multiple triggers.

## Building

Requirements:

- [Geode CLI](https://docs.geode-sdk.org/getting-started/geode-cli)
- The Geode SDK (installed via `geode sdk install`)

```sh
geode build
```

The built `.geode` file lands in `build/` and is auto-installed to GD if you've
configured a CLI profile. See the
[Geode docs](https://docs.geode-sdk.org/getting-started/create-mod/) for the
full setup.

### CI

A [GitHub Actions workflow](.github/workflows/build.yml) builds the mod on every
push (Windows, macOS, iOS, Android32, Android64) using the official
[`geode-sdk/build-geode-mod`](https://github.com/geode-sdk/build-geode-mod)
action and uploads the combined `.geode` as a build artifact.

Pushing a `v*` tag (e.g. `v0.2.0`) also creates a **GitHub Release** with the
combined `.geode` attached:

```sh
git tag v0.2.0
git push origin v0.2.0
```

> ⚠️ GitHub Actions is **disabled by default** on forks and may need to be
> enabled in the repo's **Actions** tab (and `Read and write permissions` under
> **Settings → Actions → General** → Workflow permissions) before it runs.

## How it works

### Geode mods

A Geode mod is a shared library (`.dll` / `.dylib` / `.so`) loaded by the Geode
mod loader. Each mod is described by a `mod.json` manifest (id, name, version,
dependencies, resources) and built with CMake against the Geode SDK. Entry
points are declared with `$on_mod(Loaded)`.

### Object Collab

[Object Collab](https://smjsproductions.github.io/Object-Collab/) is a framework
for registering custom objects in a way that stays compatible across mods.
Every object is registered under a string ID namespaced by the mod id (the
`_spr` suffix), which Object Collab then binds to a numeric GD object ID. When a
level is saved, it writes a small reference sheet so other players' Object
Collab installs can rebind those IDs to their own environment — avoiding
conflicts between different mod setups.

### The VHS Shader trigger

The object is a custom **trigger** (an `EffectGameObject` with the `Modifier`
game-object type, registered through Object Collab's `$object` macro). When the
trigger fires (`triggerObject` / `triggerActivated`), it shows a shared,
screen-sized `CCSprite` rendered through a custom `CCGLProgram`. The overlay
drives its own fade-in / fade-out state machine from a scheduled per-frame
`update()`, while `draw()` pushes the current time + effect parameters to the
shader as uniforms — so the scanlines, grain, tracking bands and fringing all
animate and stay independently adjustable. A `$modify` hook on
`PlayLayer::onExit` clears the shared overlay pointer so it can't dangle
between levels.
