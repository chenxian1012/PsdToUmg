# PsdToUmg

> Adobe Photoshop `.psd` → Unreal Motion Graphics `UWidgetBlueprint` importer.
> Module name internally is `PSD2UMG`; this repo is named `PsdToUmg` for clarity.

[![UE 5.7](https://img.shields.io/badge/Unreal-5.7-313131?logo=unrealengine)](https://www.unrealengine.com/)
[![Win64](https://img.shields.io/badge/platform-Win64-0078D6)]()
[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

Drop a `.psd` into the Content Browser. PsdToUmg:

- Parses every layer via vendored [MolecularMatters/psd_sdk](https://github.com/MolecularMatters/psd_sdk) (pure C++, zero external deps)
- Reads an optional `<Foo>.psd.json` sidecar for style overrides
- Generates one `UTexture2D` per raster layer (`TC_EditorIcon` / sRGB / NoMipmaps)
- Builds a `UWidgetBlueprint` with `UCanvasPanel` / `UImage` / `UButton` / `UProgressBar` /
  `UTextBlock` / `USizeBox` / `UScaleBox` / `UNamedSlot` / `UUserWidget` nodes based on
  layer-name conventions
- Supports `#linkedpsd(Avatar.psd)` to recursively import sibling PSDs as child WBPs
- Re-import works through the standard editor menu

```
PlayBtn#button_normal      → one UButton with three brushes
PlayBtn#button_hovered     ↗
PlayBtn#button_pressed     ↗
Panel#9slice(8,8,8,8)      → UImage + nine-slice margins
Avatar#linkedpsd(Avatar.psd) → UUserWidget referencing WBP_Avatar
```

## Quick start

1. Enable the plugin in your project's `.uproject`.
2. Drag a `.psd` into the Content Browser.
3. Output lands at `/Game/UI/PsdImport/<PsdName>/`.

See **[docs/schema.md](docs/schema.md)** for the layer-name tag dictionary and
sidecar JSON schema, and **[docs/samples.md](docs/samples.md)** for the test
fixture overview.

## Status

v2.0.0-rc1 — full rewrite for UE 5.7. 36/36 Automation Spec tests pass.

Known limitations (v1):
- Smart Object pixel extraction not supported (psd_sdk limitation) — use `#linkedpsd(...)` instead
- PSB (>2GB) not supported — `PsdReader` rejects with a `MessageLog` error
- 16/32-bit PSDs read but downsampled to 8-bit in the texture builder
- CommonUI `UCommonTextStyle` / `UCommonButtonStyle` are abstract in UE 5.7;
  the v1 stub builder returns empty soft paths (v2 will accept concrete overrides)
- Reimport asserts widget count stable; v1 clears and rebuilds (no in-place merge)
- Win64 editor only

## Building from source

```powershell
& "<UE_ROOT>\Engine\Build\BatchFiles\Build.bat" `
  HostProjectEditor Win64 Development `
  -Project="<this-repo>\HostProject\HostProject.uproject" -WaitMutex
```

`HostProject/Plugins/PsdToUmg` (or `PSD2UMG`) is an NTFS **junction** to the
repo root and is not tracked by git. After cloning, recreate it:

```cmd
mklink /J "HostProject\Plugins\PSD2UMG" "<absolute-path-to-repo-root>"
```

`/J` is a directory junction and does not need admin rights.

## Running the test suite

```powershell
& "<UE_ROOT>\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" `
  "<this-repo>\HostProject\HostProject.uproject" `
  -ExecCmds="Automation RunTests PSD2UMG.; Quit" `
  -unattended -nopause -NullRHI -log
```

Expected: 36/36 specs pass, exit code 0.

## Repository layout

```
PSD2UMG.uplugin                     Plugin manifest (UE 5.7, Win64)
Source/PSD2UMG/                     Editor module
  Private/Importer/                 PsdReader, PSD2UMGFactory
  Private/Schema/                   PsdDocument, NamingParser, SidecarLoader, SchemaResolver (Core-only)
  Private/Builder/                  TextureBuilder, StyleAssetBuilder, UmgBuilder
  Private/Settings/                 UDeveloperSettings
  Private/Asset/                    UPSD2UMGCache + UAssetDefinition
Source/psd_sdk/                     Vendored MolecularMatters/psd_sdk snapshot
Source/PSD2UMGTests/                Automation Spec tests
Tests/Sample/                       Six reproducible test PSDs + generate_samples.py
HostProject/                        UE project used by the test runner (not tracked entirely)
docs/                               README / schema / samples + design + plan
```

## Design + plan documents

The full design rationale and implementation plan are checked in under
`docs/superpowers/`. Useful if you want to understand why the layer-name
convention is what it is, or how the rewrite was decomposed.

## License

[MIT](LICENSE). Vendored `Source/psd_sdk/` is BSD 2-Clause © Molecular Matters
(see `Source/psd_sdk/LICENSE.psd_sdk`).
