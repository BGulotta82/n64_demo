#!/usr/bin/env python3
import struct
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ASSET_DIR = ROOT / "assets"
OUT_PATH = ASSET_DIR / "tiles.png"

# Use N64 RGB565 palette values.
# 0x0000 = transparent
# 0x03E0 = green in RGB565
# 0x001F = red in RGB565
# 0xF800 = blue in RGB565
# Add more entries here if you want a larger palette.
N64_PALETTE_565 = [
    0x0000,  # transparent
    0x03E0,  # green
]

def rgb565_to_rgb888(value: int):
    r5 = (value >> 11) & 0x1F
    g6 = (value >> 5) & 0x3F
    b5 = value & 0x1F

    r = (r5 * 255 + 15) // 31
    g = (g6 * 255 + 31) // 63
    b = (b5 * 255 + 15) // 31
    return (r, g, b)

def png_chunk(tag: bytes, data: bytes) -> bytes:
    return (
        struct.pack(">I", len(data)) +
        tag +
        data +
        struct.pack(">I", zlib.crc32(tag + data) & 0xffffffff)
    )

def build_tiles_png() -> bytes:
    # 2 tiles across, 1 tiles tall: 32x16 total
    width = 32
    height = 16

    # Left tile = index 0 (transparent)
    # Right tile = index 1 (green)
    pixels = []
    for y in range(height):
        for x in range(width):
            pixels.append(0 if x < 16 else 1)

    # Convert N64 palette to RGB888 for the PNG palette table
    palette_rgb = [rgb565_to_rgb888(v) for v in N64_PALETTE_565]

    # Build the indexed scanlines
    raw = bytearray()
    for y in range(height):
        raw.append(0)  # PNG filter type 0
        raw.extend(pixels[y * width:(y + 1) * width])

    # Palette chunk
    plte = bytearray()
    for r, g, b in palette_rgb:
        plte.extend((r, g, b))

    # Mark index 0 as transparent
    trns = bytearray([0, 255])

    png = b"\x89PNG\r\n\x1a\n"
    png += png_chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 3, 0, 0, 0))
    png += png_chunk(b"PLTE", bytes(plte))
    png += png_chunk(b"tRNS", bytes(trns))
    png += png_chunk(b"IDAT", zlib.compress(bytes(raw), 9))
    png += png_chunk(b"IEND", b"")
    return png

if __name__ == "__main__":
    ASSET_DIR.mkdir(exist_ok=True)
    OUT_PATH.write_bytes(build_tiles_png())
    print(f"Wrote {OUT_PATH}")