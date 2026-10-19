# Adding a platform

The root `Makefile` is platform-agnostic. It stages the game (unpacks a
`.love` or copies a directory, strips dev-only files) into `$(STAGE)` and then
hands over to your platform. You own everything under `platforms/<name>/`.

## Layout

```
platforms/<name>/platform.mk    required, included by the root Makefile
platforms/<name>/build.sh       optional, any helper scripts you like
```

`make <name>` appears automatically as soon as `platforms/<name>/platform.mk`
exists. No edits to the root Makefile are needed.

## Variables you receive (exported)

| Variable | Meaning |
| --- | --- |
| `GAME` | game directory or `.love` file |
| `STAGE` | staged game assets (modify freely, it's a copy) |
| `PLATFORM` | your platform name |
| `PLATFORM_DIR` | `platforms/<name>` |
| `PLATFORM_OUT` | `build/<name>`, put all your output here |
| `LUA_DIR` | Lua source tree (`$(LUA_DIR)/src` has the headers) |
| `VIRT_W/H` | resolution the game was designed for |


## Hooks you define in platform.mk

- `platform-deps`   (optional) build libraries, e.g. Lua for your toolchain
- `platform-build`  **required** compile and package into `$(PLATFORM_OUT)`
- `platform-clean`  (optional) extra cleanup (`build/` is removed anyway)
- `platform-config` (optional) print your settings for `make config`

Add recipes with `platform-build:` etc. Dependencies on files are fine too,
e.g. `platform-deps: $(MY_LUA_LIB)`.

## Conventions

- Prefix your own settings with the platform name (`ANDROID_NDK`,
  `VITA_SDK`, `KAIOS_MEM_MB`) and give them defaults with `?=` so users can
  override them in `config.mk`, the environment or the command line.
- Build Lua and other libraries under `$(PLATFORM_OUT)`, not in the source
  tree, so platforms don't overwrite each other.
- Don't touch other platforms' files.

See `platforms/kaios/` for a working example.
