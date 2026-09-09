#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ASSET_DIR = ROOT / "assets"
FILESYSTEM_DIR = ROOT / "filesystem"

def main():
    ASSET_DIR.mkdir(exist_ok=True)
    FILESYSTEM_DIR.mkdir(exist_ok=True)

    json_files = sorted(ASSET_DIR.glob("*.json"))
    if not json_files:
        print(f"No JSON files found in {ASSET_DIR}")
        return

    for json_path in json_files:
        with json_path.open("r", encoding="utf-8") as f:
            data = json.load(f)

        layers = data.get("layers", [])
        tile_layer = None
        for layer in layers:
            if layer.get("type") == "tilelayer":
                tile_layer = layer
                break

        if tile_layer is None:
            print(f"Skipping {json_path.name}: no tile layer found")
            continue

        tile_data = tile_layer.get("data", [])
        if not tile_data:
            print(f"Skipping {json_path.name}: tile layer is empty")
            continue

        out_path = FILESYSTEM_DIR / f"{json_path.stem}.bin"
        out_bytes = bytearray()

        for value in tile_data:
            if value < 0 or value > 255:
                raise ValueError(
                    f"{json_path.name}: tile id {value} is outside 0..255 and cannot be written as raw byte"
                )
            out_bytes.append(value)

        with out_path.open("wb") as f:
            f.write(out_bytes)

        print(f"Converted {json_path.name} -> {out_path} ({len(out_bytes)} bytes)")

if __name__ == "__main__":
    main()