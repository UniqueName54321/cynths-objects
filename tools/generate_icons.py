"""Generate the mod's deliberately minimal, GD-style object icons."""

from pathlib import Path
from math import cos, pi, sin

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "resources"
SCALE = 4
SIZE = 128


def p(value: float) -> int:
    return round(value * SCALE)


def canvas():
    image = Image.new("RGBA", (SIZE * SCALE, SIZE * SCALE), (0, 0, 0, 0))
    return image, ImageDraw.Draw(image)


def ellipse(draw, box, **kwargs):
    draw.ellipse(tuple(p(v) for v in box), **scaled(kwargs))


def polygon(draw, points, **kwargs):
    draw.polygon([(p(x), p(y)) for x, y in points], **scaled(kwargs))


def line(draw, points, **kwargs):
    draw.line([(p(x), p(y)) for x, y in points], **scaled(kwargs))


def scaled(kwargs):
    if "width" in kwargs:
        kwargs["width"] = p(kwargs["width"])
    return kwargs


def finish(image, name):
    image.resize((SIZE, SIZE), Image.Resampling.LANCZOS).save(OUT / name)


def orb_base(draw, color):
    dark = "#101522"
    ellipse(draw, (12, 12, 116, 116), fill=dark)
    ellipse(draw, (18, 18, 110, 110), fill=color)
    ellipse(draw, (29, 29, 99, 99), fill=dark)
    ellipse(draw, (35, 35, 93, 93), fill=color)
    return dark


def combination_orb():
    image, draw = canvas()
    cyan = "#28C9D8"
    dark = orb_base(draw, cyan)

    # Two inputs visibly converge into one output.
    line(draw, [(48, 52), (64, 68), (80, 52)], fill=dark, width=12)
    line(draw, [(48, 52), (64, 68), (80, 52)], fill="white", width=5)
    ellipse(draw, (38, 42, 54, 58), fill="white", outline=dark, width=3)
    ellipse(draw, (74, 42, 90, 58), fill="white", outline=dark, width=3)
    polygon(draw, [(64, 59), (76, 71), (64, 83), (52, 71)], fill="white", outline=dark, width=3)
    finish(image, "combination-orb.png")


def gamemode_orb():
    image, draw = canvas()
    violet = "#B44DE0"
    dark = orb_base(draw, violet)

    # A cube plus one circular switch arrow reads cleanly at 32 px.
    polygon(draw, [(51, 50), (69, 45), (81, 57), (63, 63)], fill="white", outline=dark, width=3)
    polygon(draw, [(51, 50), (63, 63), (63, 82), (51, 70)], fill="#D9D9D9", outline=dark, width=3)
    polygon(draw, [(63, 63), (81, 57), (81, 75), (63, 82)], fill="white", outline=dark, width=3)
    line(draw, [(42, 78), (38, 69), (40, 58)], fill="white", width=5)
    polygon(draw, [(34, 59), (42, 48), (47, 63)], fill="white", outline=dark, width=2)
    line(draw, [(86, 49), (91, 59), (89, 70)], fill="white", width=5)
    polygon(draw, [(95, 69), (87, 80), (82, 65)], fill="white", outline=dark, width=2)
    finish(image, "gamemode-orb.png")


def advanced_options():
    image, draw = canvas()
    green = "#2EA35B"
    dark = "#101522"

    # Trigger tile: flat green field with a single unmistakable settings gear.
    draw.rounded_rectangle((p(10), p(10), p(118), p(118)), radius=p(14), fill=dark)
    draw.rounded_rectangle((p(16), p(16), p(112), p(112)), radius=p(10), fill=green)
    teeth = []
    for i in range(16):
        radius = 34 if i % 2 == 0 else 26
        angle = -pi / 2 + i * pi / 8
        teeth.append((64 + cos(angle) * radius, 64 + sin(angle) * radius))
    polygon(draw, teeth, fill="white")
    ellipse(draw, (45, 45, 83, 83), fill="white")
    ellipse(draw, (55, 55, 73, 73), fill=green)
    finish(image, "advanced-options.png")


if __name__ == "__main__":
    combination_orb()
    gamemode_orb()
    advanced_options()
