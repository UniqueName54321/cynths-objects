# Cynth's Objects

A [Geode](https://geode-sdk.org/) mod for Geometry Dash that adds custom
objects to the editor, built on top of
[Object Collab](https://geode-sdk.org/mods/smjs.object-collab).

## Objects

| Object      | Type       | Description |
|-------------|------------|-------------|
| VHS Shader  | Decoration | Renders its sprite through a custom GLSL shader that applies a retro VHS look — chromatic aberration, scanlines, film grain and vignette. |

> Currently the pack only ships the **VHS Shader** object. More to come.

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

Adding an object looks like this:

```cpp
ObjectAPI::registerObject(ObjectInfo::builder()
    .id("my-object"_spr)
    .sprite("my-object.png"_spr)
    .construction(ComplexObject::builder()
        .factory(MyObject::create)
        .build())
    .build());
```

## License

MIT — see [LICENSE](LICENSE).
