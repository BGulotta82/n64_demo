#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ASSET_DIR = ROOT / "assets"
OUT_PATH = ASSET_DIR / "level_test.json"

W = 200
H = 15

# Initialize empty map
grid = [[0 for _ in range(W)] for _ in range(H)]

# Border walls
for y in range(H):
    for x in range(W):
        if x == 0 or x == W - 1 or y == 0 or y == H - 1:
            grid[y][x] = 1

# Platforms / ground sections
platform_rows = {
    13: [(0, 199)],
    11: [(10, 30), (50, 80), (105, 140), (160, 190)],
    9:  [(20, 45), (95, 125), (145, 175)],
    7:  [(40, 70), (110, 150), (175, 199)],
}

for row, segments in platform_rows.items():
    for start, end in segments:
        for x in range(start, end + 1):
            if 0 <= x < W and 0 <= row < H:
                grid[row][x] = 1

flat = []
for y in range(H):
    for x in range(W):
        flat.append(grid[y][x])

map_data = {
    "compressionlevel": -1,
    "height": H,
    "infinite": False,
    "layers": [{
        "data": flat,
        "height": H,
        "id": 1,
        "name": "Tile Layer 1",
        "opacity": 1,
        "type": "tilelayer",
        "visible": True,
        "width": W,
        "x": 0,
        "y": 0
    }],
    "nextlayerid": 2,
    "nextobjectid": 1,
    "orientation": "orthogonal",
    "renderorder": "right-down",
    "tiledversion": "1.10.2",
    "tileheight": 16,
    "tilesets": [],
    "tilewidth": 16,
    "type": "map",
    "version": "1.10",
    "width": W
}

ASSET_DIR.mkdir(exist_ok=True)
OUT_PATH.write_text(json.dumps(map_data, separators=(",", ":")))
print(f"Wrote {OUT_PATH} ({W}x{H})")