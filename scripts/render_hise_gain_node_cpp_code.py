from pathlib import Path

from PIL import Image, ImageDraw, ImageFont
from pygments import lex
from pygments.lexers import CppLexer
from pygments.token import Comment, Keyword, Literal, Name, Number, Operator, Punctuation, String, Token


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "articles" / "hise-dsp-buffer" / "assets" / "hise-gain-node-source.png"
OUTPUT = ROOT / "articles" / "hise-dsp-buffer" / "renders" / "hise-gain-node-cpp-code.png"


CODE = """float gain = 0.5f;

template <typename T>
void process(T& data)
{
    static constexpr int NumChannels = getFixChannelAmount();
    auto& fixData = data.template as<ProcessData<NumChannels>>();
    auto block = fixData.toAudioBlock();
    const auto numSamples = data.getNumSamples();

    for (int c = 0; c < NumChannels; ++c)
    {
        auto* samples = block.getChannelPointer(c);

        for (int i = 0; i < numSamples; ++i)
            samples[i] *= gain;
    }
}"""


def font(size: int, bold: bool = False) -> ImageFont.FreeTypeFont:
    candidates = (
        Path("C:/Windows/Fonts/CascadiaMono.ttf"),
        Path("C:/Windows/Fonts/consola.ttf"),
        Path("C:/Windows/Fonts/lucon.ttf"),
    )
    bold_candidates = (
        Path("C:/Windows/Fonts/CascadiaMonoSemiBold.ttf"),
        Path("C:/Windows/Fonts/consolab.ttf"),
    )
    for path in bold_candidates if bold else candidates:
        if path.exists():
            return ImageFont.truetype(str(path), size)

    return ImageFont.load_default()


def token_color(token: Token) -> tuple[int, int, int]:
    if token in Keyword or token in Keyword.Type:
        return (86, 156, 214)
    if token in Name.Function:
        return (220, 220, 170)
    if token in Name.Class or token in Name.Builtin:
        return (78, 201, 176)
    if token in Literal.Number or token in Number:
        return (181, 206, 168)
    if token in String:
        return (206, 145, 120)
    if token in Comment:
        return (106, 153, 85)
    if token in Operator or token in Punctuation:
        return (212, 212, 212)
    return (220, 220, 220)


def draw_code(draw: ImageDraw.ImageDraw, x: int, y: int, code: str) -> None:
    code_font = font(13)
    line_font = font(13)
    line_height = 21
    gutter_width = 37
    guide_x = x + gutter_width + 10
    text_x = guide_x + 18

    lines = code.splitlines()
    for index, line in enumerate(lines, start=1):
        baseline_y = y + (index - 1) * line_height
        draw.text(
            (x, baseline_y),
            f"{index:>2}",
            font=line_font,
            fill=(104, 119, 132),
            anchor=None,
        )
        draw.line(
            (guide_x, baseline_y - 3, guide_x, baseline_y + line_height - 3),
            fill=(34, 44, 52),
            width=1,
        )

        cursor_x = text_x
        for token, text in lex(line, CppLexer()):
            if "\n" in text:
                continue
            fill = token_color(token)
            draw.text((cursor_x, baseline_y), text, font=code_font, fill=fill)
            cursor_x += round(draw.textlength(text, font=code_font))


def main() -> None:
    width = 920
    height = 500
    divider_x = 320
    side_padding = 30

    image = Image.new("RGB", (width, height), (124, 128, 130))
    draw = ImageDraw.Draw(image)

    draw.rectangle((divider_x, 0, width, height), fill=(18, 24, 30))
    draw.rectangle((divider_x - 3, 0, divider_x + 1, height), fill=(84, 92, 98))

    node = Image.open(SOURCE).convert("RGB")
    node = node.crop((0, 0, node.width, node.height - 1))
    node_width = divider_x - side_padding * 2
    node_height = round(node.height * node_width / node.width)
    node = node.resize((node_width, node_height), Image.Resampling.LANCZOS)
    node_x = side_padding
    node_y = (height - node_height) // 2
    image.paste(node, (node_x, node_y))

    draw_code(draw, divider_x + 12, 58, CODE)

    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    image.save(OUTPUT)


if __name__ == "__main__":
    main()
