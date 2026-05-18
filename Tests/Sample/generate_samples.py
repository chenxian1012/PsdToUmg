"""
Deterministically build 6 test PSDs for PSD2UMG.

Uses psd-tools (pure Python) to write multi-layer PSDs with custom layer
names preserved. Replaces the previous ImageMagick-based generator which
merged same-bounds layers and stripped layer names.
"""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from PIL import Image, ImageDraw
from psd_tools import PSDImage
from psd_tools.api.layers import PixelLayer

SAMPLE_DIR = Path(__file__).parent


@dataclass
class LayerSpec:
    name: str
    # bounds = (left, top, right, bottom)
    bounds: tuple[int, int, int, int]
    # color = (r, g, b, a)
    color: tuple[int, int, int, int]


def _render_layer_pil(layer: LayerSpec) -> tuple[Image.Image, int, int]:
    """Render the layer rectangle as a tight PIL Image; return (img, left, top)."""
    l, t, r, b = layer.bounds
    w, h = r - l, b - t
    img = Image.new("RGBA", (w, h), layer.color)
    return img, l, t


def write_psd(name: str, canvas: tuple[int, int], layers: list[LayerSpec]) -> None:
    """Write a multi-layer PSD via psd-tools, preserving exact layer names."""
    psd = PSDImage.new(mode="RGBA", size=canvas)
    for layer in layers:
        img, left, top = _render_layer_pil(layer)
        PixelLayer.frompil(image=img, parent=psd, name=layer.name, top=top, left=left)
    out_path = SAMPLE_DIR / f"{name}.psd"
    psd.save(str(out_path))
    print(f"wrote {out_path}")


# --- Sample definitions ---------------------------------------------------

def sample_simple() -> None:
    write_psd("Simple", (1920, 1080), [
        LayerSpec("Background", (0,    0, 1920, 1080), ( 40,  40,  60, 255)),
        LayerSpec("Logo",       (50,   50,  562,  178), (200, 200, 200, 255)),
        LayerSpec("Footer",     (0, 1000, 1920, 1080), ( 20,  20,  30, 200)),
    ])


def sample_nested() -> None:
    # ImageMagick can't author PSD groups; psd-tools can with Group, but to keep
    # the sample simple and the Schema test focused, we still encode hierarchy
    # via layer names (HUD_Score, HUD_Time, Menu_Buttons_Play). PsdSchemaResolver
    # is responsible for reconstructing groups from naming if/when needed.
    write_psd("Nested", (1920, 1080), [
        LayerSpec("HUD_Score",         (50,   50,  300,  90), (255, 215,   0, 255)),
        LayerSpec("HUD_Time",          (50,  100,  300, 140), (255, 215,   0, 255)),
        LayerSpec("Menu_Buttons_Play", (800, 500, 1120, 600), (100, 200, 100, 255)),
    ])


def sample_buttons() -> None:
    write_psd("Buttons", (1920, 1080), [
        LayerSpec("PlayBtn#button_normal",  (760, 460, 1160, 620), ( 80, 140, 220, 255)),
        LayerSpec("PlayBtn#button_hovered", (760, 460, 1160, 620), (120, 180, 255, 255)),
        LayerSpec("PlayBtn#button_pressed", (760, 460, 1160, 620), ( 50, 100, 180, 255)),
    ])


def sample_nine_slice() -> None:
    write_psd("NineSlice", (1920, 1080), [
        LayerSpec("Panel#9slice(8,8,8,8)", (200, 200, 1720, 880), (60, 60, 80, 220)),
    ])


def sample_linked_psd() -> None:
    # Parent PSD: one layer whose name carries the #linkedpsd tag pointing at
    # sibling Avatar.psd. The rectangle is the placeholder bounds for the
    # UserWidget that will be generated at this position.
    write_psd("LinkedPsd", (1920, 1080), [
        LayerSpec("Avatar#linkedpsd(Avatar.psd)", (832, 412, 1088, 668), (40, 40, 60, 1)),
    ])
    # Child PSD: a tiny standalone PSD that #linkedpsd refers to.
    write_psd("Avatar", (256, 256), [
        LayerSpec("Body", (0, 0, 256, 256), (200, 120, 60, 255)),
    ])


if __name__ == "__main__":
    sample_simple()
    sample_nested()
    sample_buttons()
    sample_nine_slice()
    sample_linked_psd()
