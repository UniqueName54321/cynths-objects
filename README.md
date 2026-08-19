# Cynth's Objects

A [Geode](https://geode-sdk.org/) mod for Geometry Dash that adds custom
objects to the editor, built on top of
[Object Collab](https://geode-sdk.org/mods/smjs.object-collab).

## Objects

| Object       | Type       | Description |
|--------------|------------|-------------|
| VHS Overlay  | Decoration | A full-screen VHS-style shader overlay — scanlines, film grain, vignette, color fringing and tracking lines. Adjustable opacity via its "Edit Special" screen. |

> Currently the pack only ships the **VHS Overlay** object. More to come.

## Usage

1. Place the **VHS Overlay** object anywhere in the level.
2. Select it and open **Edit Special** (the wrench) to set the **Opacity**
   (0 = invisible, 255 = fully opaque; default is 100).
3. Play the level — a full-screen VHS effect renders over the whole scene.

> Only one overlay is shown at a time, even if you place multiple objects.

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

Pushing a `v*` tag (e.g. `v0.1.0`) also creates a **GitHub Release** with the
combined `.geode` attached:

```sh
git tag v0.1.0
git push origin v0.1.0
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

### The VHS Overlay

The object is a normal custom object (a decoration), but it uses its
`postPlayLayerInit()` hook to spawn a separate full-screen `CCSprite` onto the
`PlayLayer`, scaled to the window size and rendered through a custom
`CCGLProgram`. The shader only synthesizes the VHS effect (scanlines, grain,
vignette, fringing, tracking bands) and relies on normal alpha blending to
composite over the scene, so **opacity** maps directly to how strongly the
effect shows. A `$modify` hook on `PlayLayer::onExit` clears the shared overlay
pointer so it can't dangle between levels.

## License

MIT — see [LICENSE](LICENSE).
