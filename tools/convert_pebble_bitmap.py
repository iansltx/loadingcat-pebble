#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image

PEBBLE_LEVELS = (0, 85, 170, 255)


def build_palette() -> list[int]:
    values: list[int] = []
    for r in PEBBLE_LEVELS:
        for g in PEBBLE_LEVELS:
            for b in PEBBLE_LEVELS:
                values.extend((r, g, b))
    values.extend([0] * (256 * 3 - len(values)))
    return values


def quantize_channel(value: int) -> int:
    return max(0, min(3, int(round(value / 85.0))))


def composite_rgba(image: Image.Image) -> Image.Image:
    if image.mode != "RGBA":
        return image.convert("RGB")

    background = Image.new("RGBA", image.size, image.getpixel((0, 0)))
    return Image.alpha_composite(background, image).convert("RGB")


def convert_to_pebble_bitmap(src: Path, dst: Path, size: tuple[int, int]) -> None:
    image = Image.open(src)
    image = composite_rgba(image)
    if image.size != size:
        image = image.resize(size, Image.Resampling.NEAREST)

    indexed = Image.new("P", image.size)
    indexed.putpalette(build_palette())

    palette_indexes: list[int] = []
    pixels = image.load()
    for y in range(image.height):
        for x in range(image.width):
            r, g, b = pixels[x, y]
            ri = quantize_channel(r)
            gi = quantize_channel(g)
            bi = quantize_channel(b)
            palette_indexes.append((ri * 16) + (gi * 4) + bi)

    indexed.putdata(palette_indexes)
    dst.parent.mkdir(parents=True, exist_ok=True)
    indexed.save(dst, optimize=False)


def main() -> None:
    parser = argparse.ArgumentParser(description="Convert an image to a Pebble 64-color bitmap PNG")
    parser.add_argument("src", type=Path)
    parser.add_argument("dst", type=Path)
    parser.add_argument("--width", type=int, default=200)
    parser.add_argument("--height", type=int, default=228)
    args = parser.parse_args()

    convert_to_pebble_bitmap(args.src, args.dst, (args.width, args.height))


if __name__ == "__main__":
    main()
