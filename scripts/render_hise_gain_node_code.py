from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "articles" / "hise-dsp-buffer" / "assets" / "hise-gain-node-source.png"
OUTPUT = ROOT / "articles" / "hise-dsp-buffer" / "renders" / "hise-gain-node-code.png"


def font(size: int) -> ImageFont.FreeTypeFont:
    for path in (
        Path("C:/Windows/Fonts/consola.ttf"),
        Path("C:/Windows/Fonts/CascadiaMono.ttf"),
        Path("C:/Windows/Fonts/lucon.ttf"),
    ):
        if path.exists():
            return ImageFont.truetype(str(path), size)

    return ImageFont.load_default()


def main() -> None:
    width = 920
    height = 360
    divider_x = 460
    side_padding = 55

    image = Image.new("RGB", (width, height), (29, 29, 29))
    draw = ImageDraw.Draw(image)

    draw.rectangle((divider_x, 0, width, height), fill=(18, 24, 30))
    draw.rectangle((divider_x - 3, 0, divider_x + 1, height), fill=(43, 54, 62))

    node = Image.open(SOURCE).convert("RGB")
    node = node.crop((0, 0, node.width, node.height - 1))
    node_width = divider_x - side_padding * 2
    node_height = round(node.height * node_width / node.width)
    node = node.resize((node_width, node_height), Image.Resampling.LANCZOS)
    node_x = side_padding
    node_y = (height - node_height) // 2
    image.paste(node, (node_x, node_y))

    code_font = font(21)
    code_x = divider_x + 60
    code_y = 136
    line_gap = 42
    draw.text(
        (code_x, code_y),
        "for each sample in the buffer:",
        font=code_font,
        fill=(227, 234, 239),
    )
    draw.text(
        (code_x, code_y + line_gap),
        "sample = sample * gain",
        font=code_font,
        fill=(154, 188, 255),
    )

    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    image.save(OUTPUT)


if __name__ == "__main__":
    main()
