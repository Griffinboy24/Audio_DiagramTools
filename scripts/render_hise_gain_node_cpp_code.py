from pathlib import Path

from PIL import Image, ImageDraw, ImageFont
from pygments import lex
from pygments.lexers import CppLexer
from pygments.token import Comment, Keyword, Literal, Name, Number, Operator, Punctuation, String, Token


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "articles" / "hise-dsp-buffer" / "assets" / "hise-gain-node-source.png"
OUTPUT = ROOT / "articles" / "hise-dsp-buffer" / "renders" / "hise-gain-node-cpp-code.png"


CODE = """void setGain(float newGain)
{
    gain = newGain;
}

void process(chunk)
{
    // Ask the chunk how many sample values it contains
    const int numSamples = chunk.getNumSamples();

    // Run the same process for each audio channel
    for (int channel = 0; channel < 2; ++channel)
    {
        // Get the row of samples for this channel
        auto* samples = chunk.getChannel(channel);

        // Move through that row one sample at a time
        for (int s = 0; s < numSamples; ++s)
        {
            // Read the current sample value
            const float input = samples[s];

            // Write it back, scaled by the gain amount
            samples[s] = input * gain;
        }
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


def draw_code(
    draw: ImageDraw.ImageDraw,
    x: int,
    y: int,
    code: str,
    canvas_height: int,
) -> None:
    code_font = font(12)
    line_font = font(12)
    line_height = 17
    gutter_width = 38
    text_x = x + gutter_width + 8
    char_width = draw.textlength(" ", font=code_font)
    start_line = 858

    lines = code.splitlines()
    for line_number in range(start_line - 4, start_line + len(lines) + 5):
        baseline_y = y + (line_number - start_line) * line_height
        if -line_height < baseline_y < canvas_height:
            draw.text(
                (x, baseline_y),
                f"{line_number:>3}",
                font=line_font,
                fill=(104, 119, 132),
            )

    indent_guides: list[tuple[int, int, int]] = []
    stack: list[tuple[int, int]] = []
    for index, line in enumerate(lines):
        leading_spaces = len(line) - len(line.lstrip(" "))
        if "}" in line and stack:
            guide_x, start_index = stack.pop()
            indent_guides.append((guide_x, start_index, index))
        if "{" in line:
            stack.append((round(text_x + leading_spaces * char_width), index))

    for guide_x, start_index, end_index in indent_guides:
        draw.line(
            (
                guide_x,
                y + (start_index + 1) * line_height - 2,
                guide_x,
                y + end_index * line_height - 2,
            ),
            fill=(42, 52, 60),
            width=1,
        )

    for index, line in enumerate(lines, start=1):
        baseline_y = y + (index - 1) * line_height
        cursor_x = text_x
        for token, text in lex(line, CppLexer()):
            if "\n" in text:
                text = text.replace("\n", "")
                if not text:
                    continue
            fill = token_color(token)
            draw.text((cursor_x, baseline_y), text, font=code_font, fill=fill)
            cursor_x += round(draw.textlength(text, font=code_font))


def main() -> None:
    width = 920
    height = 500
    divider_x = 460
    side_padding = 55

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

    draw_code(draw, divider_x + 18, 20, CODE, height)

    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    image.save(OUTPUT)


if __name__ == "__main__":
    main()
