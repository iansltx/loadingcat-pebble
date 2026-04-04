#!/usr/bin/env python3

from pathlib import Path

from PIL import Image, ImageFilter


ROOT = Path(__file__).resolve().parents[1]
SOURCE_PATH = ROOT / "resources" / "images" / "loadingcat_bg.png"
OUTPUT_DIR = ROOT / "resources" / "images"

RECT_BOX = (2, 0, 197, 228)
ROUND_BOX = (0, 8, 200, 208)

PLATFORM_CONFIGS = {
    "aplite": {"size": (144, 168), "box": RECT_BOX, "bw": True},
    "basalt": {"size": (144, 168), "box": RECT_BOX, "bw": False},
    "chalk": {"size": (180, 180), "box": ROUND_BOX, "bw": False},
    "diorite": {"size": (144, 168), "box": RECT_BOX, "bw": True},
    "flint": {"size": (144, 168), "box": RECT_BOX, "bw": False},
    "gabbro": {"size": (260, 260), "box": ROUND_BOX, "bw": False},
}


def crop_resize(image: Image.Image, box: tuple[int, int, int, int],
                size: tuple[int, int]) -> Image.Image:
    return image.crop(box).resize(size, Image.Resampling.NEAREST)


def build_ear_mask(image: Image.Image) -> Image.Image:
    mask = Image.new("L", image.size, 0)
    src = image.load()
    dst = mask.load()
    for y in range(image.height):
        for x in range(image.width):
            r, g, b, a = src[x, y]
            if a and r >= 95 and r >= g + 22 and r >= b + 22:
                dst[x, y] = 255
    return mask


def to_bw(image: Image.Image, ear_mask: Image.Image) -> Image.Image:
    gray = image.convert("L")
    boosted_ears = ear_mask.filter(ImageFilter.MaxFilter(3))
    output = Image.new("1", image.size, 1)
    gray_px = gray.load()
    ear_px = boosted_ears.load()
    out_px = output.load()

    for y in range(image.height):
        for x in range(image.width):
            out_px[x, y] = 255 if gray_px[x, y] >= 122 or ear_px[x, y] else 0

    return output.convert("RGBA")


def main() -> None:
    source = Image.open(SOURCE_PATH).convert("RGBA")
    ear_mask = build_ear_mask(source)

    for platform, config in PLATFORM_CONFIGS.items():
        size = config["size"]
        box = config["box"]
        platform_image = crop_resize(source, box, size)

        if config["bw"]:
            platform_mask = crop_resize(ear_mask, box, size)
            platform_image = to_bw(platform_image, platform_mask)

        out_path = OUTPUT_DIR / f"loadingcat_bg~{platform}.png"
        platform_image.save(out_path)
        print(f"wrote {out_path.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
