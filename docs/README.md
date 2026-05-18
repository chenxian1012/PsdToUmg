# PSD2UMG

Adobe Photoshop `.psd` → Unreal Motion Graphics `UWidgetBlueprint` importer for **Unreal Engine 5.7**.

## What it does

Drop a `.psd` file into the Content Browser. PSD2UMG:

- Parses every layer via MolecularMatters/psd_sdk (vendored, pure C++, zero external deps)
- Reads optional `<Foo>.psd.json` sidecar for style overrides
- Generates one `UTexture2D` per raster layer (TC_EditorIcon / sRGB / NoMipmaps)
- Builds a `UWidgetBlueprint` with `UCanvasPanel` / `UImage` / `UButton` / `UProgressBar` /
  `UTextBlock` / `USizeBox` / `UScaleBox` / `UNamedSlot` / `UUserWidget` nodes based on
  layer-name conventions (see `schema.md`)
- Re-import is supported via the standard Reimport menu

See `docs/schema.md` for the layer-name protocol and `docs/samples.md` for the test fixtures.

## Quick start

1. Enable the plugin in your project's `.uproject`.
2. Right-click a `.psd` in the Content Browser → Import.
3. The output lands at `/Game/UI/PsdImport/<PsdName>/`:
   - `WBP_<PsdName>.uasset` — the WidgetBlueprint
   - `T_<LayerName>.uasset` — per-layer texture
   - `PsdCache_<PsdName>.uasset` — Reimport handle

Use **Project Settings → Plugins → PSD2UMG** to change the default output path
or disable CommonUI integration.

## Repository layout

```
PSD2UMG.uplugin                            Plugin manifest (UE 5.7, Win64)
Source/PSD2UMG/                            Editor module (Importer/Schema/Builder/Settings/Asset)
Source/psd_sdk/                            Vendored MolecularMatters/psd_sdk snapshot
Source/PSD2UMGTests/                       Automation Spec tests (DeveloperTool)
Tests/Sample/                              Reproducible test fixtures (generate_samples.py)
HostProject/                               UE project used by the test runner
docs/                                      This documentation
```

## Building from source

```bash
"D:\ue\UE_5.7\Engine\Build\BatchFiles\Build.bat" \
  HostProjectEditor Win64 Development \
  -Project="D:\Ai\Project\PSD2UMG_5.7\HostProject\HostProject.uproject" \
  -WaitMutex
```

The plugin compiles two UE modules into the host project: `psd_sdk` (the vendored
parser) and `PSD2UMG` (the editor integration). A third module `PSD2UMGTests`
ships the Automation Spec suite.

## Running the test suite

```bash
"D:\ue\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" \
  "D:\Ai\Project\PSD2UMG_5.7\HostProject\HostProject.uproject" \
  -ExecCmds="Automation RunTests PSD2UMG.; Quit" \
  -unattended -nopause -NullRHI -log
```

Expected: 36/36 specs pass, exit code 0.

## Repo bootstrap (clone-time setup)

The host project's `HostProject/Plugins/PSD2UMG` is an NTFS **junction** pointing to the
repo root — not a tracked file. After cloning the repo, recreate it:

```cmd
mklink /J "HostProject\Plugins\PSD2UMG" ".\"
```

(Requires Developer Mode on Windows, OR an admin prompt. `/J` is a junction, which
doesn't need elevation.)

## Known limitations (v1)

- **Smart Object pixel extraction** is not supported by psd_sdk. Use the
  `#linkedpsd(SubFile.psd)` layer-name convention to embed sibling PSDs as
  child WBPs instead. See `schema.md`.
- **PSB (>2GB) files** are not supported. PSD Reader rejects them with a
  MessageLog error.
- **16/32-bit PSDs** are read but downsampled to 8-bit in the texture builder.
  v2 will preserve precision with `TSF_RGBA16F`.
- **Style assets** (`CommonTextStyle` / `CommonButtonStyle`) are abstract in
  CommonUI 5.7; the v1 stub builder returns empty soft paths. v2 will accept
  concrete subclass overrides via project settings.
- **Reimport idempotence** asserts widget COUNT is stable, not in-place merge.
  v1 clears and rebuilds the WBP; user-edited Slot positions are not preserved.

## License

See `Source/psd_sdk/LICENSE.psd_sdk` for the vendored parser (BSD 2-Clause,
© Molecular Matters). The plugin glue code license is per-repo (specify in your project).
