"""
Deterministically build 6 test PSDs and their expected.json metadata.

Pipeline per sample:
  1. Render each layer as a transparent PNG (Pillow).
  2. Call `magick LAYER_PNGS -compose Over OUT.psd` so each PNG becomes a
     Photoshop layer.
  3. Write <Sample>.expected.json with the layer metadata C++ tests assert on.

Limitations: ImageMagick exports flat layered PSDs; groups and smart objects
are not representable this way. `Nested.psd` uses layer-name conventions
to encode hierarchy. The `LinkedPsd` sample uses the `#linkedpsd(Avatar.psd)`
naming convention pointing to the sibling `Avatar.psd` file.
"""

from __future__ import annotations
import json
import subprocess
from dataclasses import dataclass
from pathlib import Path
from PIL import Image, ImageDraw

SAMPLE_DIR = Path(__file__).parent


@dataclass
class LayerSpec:
    name: str
    bounds: tuple[int, int, int, int]   # left, top, right, bottom
    color: tuple[int, int, int, int]    # RGBA


def render_layer(canvas_w: int, canvas_h: int, layer: LayerSpec) -> Image.Image:
    img = Image.new("RGBA", (canvas_w, canvas_h), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.rectangle(layer.bounds, fill=layer.color)
    return img


def build_flat_psd(name: str, canvas_w: int, canvas_h: int, layers: list[LayerSpec]) -> None:
    out_psd = SAMPLE_DIR / f"{name}.psd"
    pngs: list[Path] = []
    for layer in layers:
        # Sanitize layer name for filename (replace # ( ) , with _)
        safe = layer.name.replace("#", "_").replace("(", "_").replace(")", "_").replace(",", "_")
        p = SAMPLE_DIR / f"_tmp_{name}_{safe}.png"
        render_layer(canvas_w, canvas_h, layer).save(p)
        pngs.append(p)

    cmd = ["magick"] + [str(p) for p in pngs] + ["-compose", "Over", str(out_psd)]
    subprocess.run(cmd, check=True)
    for p in pngs:
        p.unlink()

    print(f"wrote {out_psd}")


def write_expected_json(name: str, canvas: tuple[int, int],
                         layers: list[LayerSpec], color_depth: int = 8) -> None:
    data = {
        "canvasSize": list(canvas),
        "colorDepth": color_depth,
        "layerCount": len(layers),
        "layers": [
            {"name": l.name, "kind": "Raster", "bounds": list(l.bounds)}
            for l in layers
        ],
    }
    (SAMPLE_DIR / f"{name}.expected.json").write_text(json.dumps(data, indent=2))


# --- Sample definitions ---------------------------------------------------

def sample_simple() -> None:
    layers = [
        LayerSpec("Background", (0,    0, 1920, 1080), (40,  40,  60, 255)),
        LayerSpec("Logo",       (50,   50,  562,  178), (200, 200, 200, 255)),
        LayerSpec("Footer",     (0, 1000, 1920, 1080), (20,  20,  30, 200)),
    ]
    build_flat_psd("Simple", 1920, 1080, layers)
    write_expected_json("Simple", (1920, 1080), layers)


def sample_nested() -> None:
    layers = [
        LayerSpec("HUD_Score",          (50,   50,  300,  90),  (255, 215, 0,   255)),
        LayerSpec("HUD_Time",           (50,  100,  300, 140),  (255, 215, 0,   255)),
        LayerSpec("Menu_Buttons_Play",  (800, 500, 1120, 600),  (100, 200, 100, 255)),
    ]
    build_flat_psd("Nested", 1920, 1080, layers)
    write_expected_json("Nested", (1920, 1080), layers)


def sample_buttons() -> None:
    layers = [
        LayerSpec("PlayBtn#button_normal",  (760, 460, 1160, 620), ( 80, 140, 220, 255)),
        LayerSpec("PlayBtn#button_hovered", (760, 460, 1160, 620), (120, 180, 255, 255)),
        LayerSpec("PlayBtn#button_pressed", (760, 460, 1160, 620), ( 50, 100, 180, 255)),
    ]
    build_flat_psd("Buttons", 1920, 1080, layers)
    write_expected_json("Buttons", (1920, 1080), layers)


def sample_nine_slice() -> None:
    layers = [
        LayerSpec("Panel#9slice(8,8,8,8)", (200, 200, 1720, 880), (60, 60, 80, 220)),
    ]
    build_flat_psd("NineSlice", 1920, 1080, layers)
    write_expected_json("NineSlice", (1920, 1080), layers)


def sample_linked_psd() -> None:
    """Parent PSD that references Avatar.psd via #linkedpsd naming convention."""
    parent_layers = [
        LayerSpec("Avatar#linkedpsd(Avatar.psd)", (832, 412, 1088, 668), (40, 40, 60, 1)),
    ]
    build_flat_psd("LinkedPsd", 1920, 1080, parent_layers)
    write_expected_json("LinkedPsd", (1920, 1080), parent_layers)

    child_layers = [
        LayerSpec("Body", (0, 0, 256, 256), (200, 120, 60, 255)),
    ]
    build_flat_psd("Avatar", 256, 256, child_layers)
    write_expected_json("Avatar", (256, 256), child_layers)


if __name__ == "__main__":
    sample_simple()
    sample_nested()
    sample_buttons()
    sample_nine_slice()
    sample_linked_psd()
