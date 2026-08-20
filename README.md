# Cynth's Objects

A [Geode](https://geode-sdk.org/) mod for Geometry Dash that adds custom
objects to the editor, built on top of
[Object Collab](https://geode-sdk.org/mods/smjs.object-collab).

## Objects

| Object                  | Type    | Description |
|-------------------------|---------|-------------|
| Advanced Options Trigger | Trigger | Changes one or more level/player settings at once. Includes 172 options across 6 categories and a searchable browser in Edit Object. |

## Usage

1. Place the **Advanced Options Trigger** in the level.
2. Select it and open **Edit Object**. Use the touch/spawn/multi
   trigger toggles to decide how it activates.
3. Click **Browse Options...** to open the searchable browser, then type to
   filter by name, category, or key number.
4. Select a row, enable it, and set its value in the browser. Repeat for every
   setting the trigger should change; the browser stays open between rows.
5. Play the level — triggering it applies the setting to the current level
   (or to player 1).

Each trigger instance can change **multiple settings** in one activation.

### Option browser legend

| Marker | Meaning |
|--------|---------|
| `[*]`  | Takes effect instantly |
| `[!]`  | Needs a respawn/restart |

## Options

The trigger covers **172** options across 6 categories. Keys are the internal
IDs shown in the browser (e.g. `#7`, `#150`).

| Category        | Key range   | Count |
|-----------------|-------------|-------|
| Level Settings  | 0–15        | 16    |
| Gameplay        | 30–56       | 27    |
| Camera          | 60–69       | 10    |
| Physics         | 80–91       | 12    |
| Visual / Debug  | 100–115     | 16    |
| Player          | 130–220     | 91    |
| **Total**       |             | **172** |

### Value types

The **Value** slider maps differently depending on the chosen setting:

- **Bool** — slider `0→0.5` = OFF, `0.5→1` = ON
- **Speed** — `0`=Slow, `0.25`=Normal, `0.5`=Fast, `0.75`=Very Fast, `1`=Extreme
- **Mode** — `0`=Cube, `0.14`=Ship, `0.29`=Ball, `0.43`=UFO, `0.57`=Wave,
  `0.71`=Robot, `0.86`=Spider, `1`=Swing
- **Int (0–100)** — slider maps to 0–100
- **Int999 (0–999)** — slider maps to 0–999 (spawn group, target, icon request)
- **Float** — slider maps to the setting's documented range

All **Player** options apply to player 1.

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

Pushing a `v*` tag (e.g. `v0.5.2`) also creates a **GitHub Release** with the
combined `.geode` attached:

```sh
git tag v0.5.2
git push origin v0.5.2
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

### The Advanced Options Trigger

The object is a custom **trigger** (an `EffectGameObject` with the `Modifier`
game-object type, registered through Object Collab's `$object` macro). Each
instance stores a sparse list of enabled option/value pairs as a custom
property. The legacy **option key** and **value** properties remain readable so
levels made with v0.5.1 continue to work. When the trigger fires (`triggerObject`
/ `triggerActivated`), it maps each enabled value to the appropriate type (Bool
/ Speed / Mode / Int / Float) and applies it to `PlayLayer`,
`LevelSettingsObject`, or `PlayerObject`.

The **Edit Object** screen is defined with Object Collab's `PopupConfig`. A
`CustomValueMenu` renders the **Browse Options...** button. The browser uses a
`TextInput`, a `ScrollLayer`, and value controls to search, enable, disable, and
configure several settings without closing the popup.
