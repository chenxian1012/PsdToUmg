# PSD Layer Naming + Sidecar Schema

PSD2UMG drives widget generation from two sources:

1. **Layer name tags** (the primary mechanism; designers control this in Photoshop)
2. **Sidecar `<Foo>.psd.json`** (the optional mechanism; controls style asset references)

## Layer name format

```
<Name>#<tag1>[(args)]#<tag2>[(args)]...
```

`<Name>` becomes the widget's `FName` in the generated WidgetBlueprint. Tags
following `#` add type, modifiers, or behavior. Multiple tags compose.

### Tag dictionary (v1)

| Tag | Output widget | Args |
|---|---|---|
| `#image` (default) | `UImage` | — |
| `#text` | `UTextBlock` (or `UCommonTextBlock` with CommonUI) | — |
| `#button` | `UButton` (or `UCommonButtonBase`) | — |
| `#button_normal` / `#button_hovered` / `#button_pressed` / `#button_disabled` | merges into one `UButton` with per-state brushes | — |
| `#progress(bg\|fill\|marquee)` | `UProgressBar` part | — |
| `#9slice(L,R,T,B)` | nine-slice border on `UImage` brush; values are pixel margins | `8,8,8,8` |
| `#sizebox(W=,H=)` | `USizeBox` with width/height override | `W=200,H=80` |
| `#scalebox` | `UScaleBox` | — |
| `#slot` | `UNamedSlot` (fill at runtime) | — |
| `#anchor(...)` | overrides default anchor | one of `TL / T / TR / L / C / R / BL / B / BR / Stretch` |
| `#linkedpsd(RelPath)` | sibling `.psd` becomes a child `WBP_<base>`; embedded as a `UUserWidget` | `Avatar.psd` |
| `#vanilla` | forces plain UMG (no CommonUI) | — |
| `#skip` | excluded from output | — |

Unknown tags are recorded as MessageLog warnings and treated as no-ops.

### Default anchor preset

When `#anchor(...)` is not specified, PSD2UMG derives an anchor from the layer's
center position within its parent (the PSD canvas):

| X / Y in parent | Result |
|---|---|
| < 25% | start-aligned anchor (TL/T/L/etc.) |
| 25–75% | center-aligned |
| > 75% | end-aligned |

This is a 9-grid approximation. Override with `#anchor(...)` when the auto-detection is wrong.

## Sidecar `<Foo>.psd.json`

If a JSON file named `<Foo>.psd.json` sits next to `<Foo>.psd`, PSD2UMG loads it
and lets values inside override defaults derived from layer names.

```json
{
  "version": 1,
  "globals": {
    "designDpi": 1920,
    "useCommonUI": true
  },
  "layers": {
    "PlayBtn": {
      "commonButtonStyle": "/Game/UI/Styles/BSt_Primary.BSt_Primary"
    },
    "Title": {
      "textStyle": "/Game/UI/Styles/TS_Title.TS_Title",
      "fontFace": "/Game/Fonts/MainFont.MainFont"
    }
  }
}
```

### Override priority

`JSON > naming tag args > UDeveloperSettings defaults`

Only assets referenced via `commonButtonStyle` / `textStyle` / `fontFace` need
to exist in the project — if missing, the import logs a warning and continues
with default styling.

## Examples

| PSD layer name | Resulting widget |
|---|---|
| `Background` | `UImage` named `Background`, anchor auto-derived |
| `PlayBtn#button_normal` + `PlayBtn#button_hovered` + `PlayBtn#button_pressed` | One `UButton` named `PlayBtn`, three brushes set |
| `Panel#9slice(8,8,8,8)` | `UImage` with `Margin{L:8,R:8,T:8,B:8}` and `DrawAs=Box` |
| `Avatar#linkedpsd(Avatar.psd)` | `UUserWidget` referencing `WBP_Avatar` (also imported recursively) |
| `Title#text#vanilla` | `UTextBlock` (not the CommonUI version) |
| `Header#slot` | `UNamedSlot` for runtime fill |
