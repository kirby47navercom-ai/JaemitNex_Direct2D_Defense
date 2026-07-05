from __future__ import annotations

import math
import random
import struct
import wave
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageEnhance, ImageFilter


ROOT = Path(__file__).resolve().parents[1]
BACKGROUND_DIR = ROOT / "assets" / "backgrounds"
CUTIN_DIR = ROOT / "assets" / "cutins"
UI_DIR = ROOT / "assets" / "ui"
VFX_DIR = ROOT / "assets" / "vfx"
MUSIC_DIR = ROOT / "assets" / "music"

STAGE_NAMES = [
    "mercury",
    "venus",
    "earth",
    "mars",
    "jupiter",
    "saturn",
    "uranus",
    "neptune",
    "pluto",
    "sun",
]

STAGE_PALETTES = [
    ((32, 37, 44), (108, 119, 128), (210, 220, 226)),
    ((58, 38, 32), (214, 132, 82), (255, 214, 124)),
    ((20, 52, 70), (62, 143, 118), (158, 224, 255)),
    ((58, 28, 32), (178, 80, 64), (255, 145, 92)),
    ((64, 44, 32), (203, 152, 90), (255, 220, 150)),
    ((54, 48, 38), (201, 180, 105), (246, 232, 170)),
    ((28, 55, 60), (102, 220, 210), (210, 255, 250)),
    ((20, 34, 72), (82, 132, 238), (190, 216, 255)),
    ((38, 34, 60), (160, 136, 220), (235, 230, 255)),
    ((86, 45, 18), (255, 160, 58), (255, 240, 172)),
]


def ensure_dirs() -> None:
    for directory in (BACKGROUND_DIR, CUTIN_DIR, UI_DIR, VFX_DIR, MUSIC_DIR):
        directory.mkdir(parents=True, exist_ok=True)


def add_blurred_ellipse(layer: Image.Image, bbox: tuple[int, int, int, int], color: tuple[int, int, int, int], blur: float) -> None:
    glow = Image.new("RGBA", layer.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(glow, "RGBA")
    draw.ellipse(bbox, fill=color)
    glow = glow.filter(ImageFilter.GaussianBlur(blur))
    layer.alpha_composite(glow)


def make_space_background(stage: int, path: Path, size: tuple[int, int] = (2560, 1440)) -> None:
    rng = random.Random(9400 + stage * 73)
    width, height = size
    dark, mid, bright = STAGE_PALETTES[stage]

    noise = Image.effect_noise(size, 96).convert("L")
    noise = ImageEnhance.Contrast(noise).enhance(1.8)
    base = Image.new("RGB", size, dark)
    tint = Image.new("RGB", size, mid)
    base = Image.blend(base, tint, 0.18)

    color_noise = Image.merge(
        "RGB",
        (
            noise.point(lambda p: int(p * (0.18 + stage * 0.006))),
            noise.point(lambda p: int(p * 0.16)),
            noise.point(lambda p: int(p * (0.22 + (9 - stage) * 0.005))),
        ),
    )
    base = ImageChops.screen(base, color_noise)
    layer = base.convert("RGBA")

    for _ in range(10):
        cx = rng.randint(-240, width + 240)
        cy = rng.randint(-180, height + 180)
        rx = rng.randint(260, 820)
        ry = rng.randint(90, 320)
        color = rng.choice((mid, bright, (120, 160, 220), (255, 190, 120)))
        add_blurred_ellipse(
            layer,
            (cx - rx, cy - ry, cx + rx, cy + ry),
            (color[0], color[1], color[2], rng.randint(22, 58)),
            rng.uniform(36, 92),
        )

    draw = ImageDraw.Draw(layer, "RGBA")
    for _ in range(1700):
        x = rng.randrange(width)
        y = rng.randrange(height)
        power = rng.random()
        radius = 1 if power < 0.86 else rng.choice((2, 2, 3))
        alpha = int(80 + power * 175)
        color = rng.choice(((235, 248, 255), bright, (255, 230, 180)))
        draw.ellipse((x - radius, y - radius, x + radius, y + radius), fill=(color[0], color[1], color[2], alpha))
        if power > 0.985:
            draw.line((x - 12, y, x + 12, y), fill=(color[0], color[1], color[2], 65), width=1)
            draw.line((x, y - 8, x, y + 8), fill=(color[0], color[1], color[2], 45), width=1)

    for _ in range(18):
        x = rng.randint(-120, width + 120)
        y = rng.randint(-80, height + 80)
        length = rng.randint(180, 520)
        tilt = rng.uniform(-0.45, 0.45)
        color = rng.choice((bright, mid, (140, 180, 255)))
        draw.line((x, y, x + length, y + int(length * tilt)), fill=(color[0], color[1], color[2], rng.randint(20, 54)), width=rng.randint(1, 3))

    if stage in (5, 9):
        cx = int(width * (0.72 if stage == 5 else 0.20))
        cy = int(height * (0.25 if stage == 5 else 0.72))
        add_blurred_ellipse(layer, (cx - 420, cy - 120, cx + 420, cy + 120), (bright[0], bright[1], bright[2], 70), 42)
        draw.arc((cx - 460, cy - 130, cx + 460, cy + 130), 178, 356, fill=(bright[0], bright[1], bright[2], 96), width=3)

    layer = layer.filter(ImageFilter.UnsharpMask(radius=1.6, percent=115, threshold=4))
    layer.convert("RGB").save(path, "JPEG", quality=96, subsampling=0, optimize=True)


def make_cutin_art() -> None:
    make_space_background(8, CUTIN_DIR / "prologue_story_art.jpg", (2560, 1440))
    prologue = Image.open(CUTIN_DIR / "prologue_story_art.jpg").convert("RGBA")
    draw = ImageDraw.Draw(prologue, "RGBA")
    for i in range(12):
        y = 980 - i * 58
        alpha = max(12, 74 - i * 5)
        draw.line((160, y, 2400, y - 220), fill=(246, 255, 131, alpha), width=4)
    for i in range(10):
        t = i / 9
        x = 280 + int(t * 1980)
        y = 1040 + int(math.sin(t * math.pi * 2) * 76)
        color = STAGE_PALETTES[i][2]
        draw.ellipse((x - 22, y - 22, x + 22, y + 22), fill=(color[0], color[1], color[2], 205), outline=(255, 255, 255, 140), width=3)
    prologue.convert("RGB").save(CUTIN_DIR / "prologue_story_art.jpg", "JPEG", quality=96, subsampling=0, optimize=True)

    make_space_background(9, CUTIN_DIR / "ending_solar_route.jpg", (2560, 1440))
    ending = Image.open(CUTIN_DIR / "ending_solar_route.jpg").convert("RGBA")
    draw = ImageDraw.Draw(ending, "RGBA")
    for radius, alpha in ((520, 46), (360, 72), (210, 120)):
        add_blurred_ellipse(ending, (1280 - radius, 440 - radius, 1280 + radius, 440 + radius), (255, 178, 70, alpha), radius * 0.18)
    draw.ellipse((1218, 378, 1342, 502), fill=(255, 238, 178, 230))
    for i in range(24):
        angle = i / 24 * math.tau
        draw.line((1280 + math.cos(angle) * 120, 440 + math.sin(angle) * 120,
                   1280 + math.cos(angle) * 520, 440 + math.sin(angle) * 520),
                  fill=(255, 190, 90, 84), width=5)
    draw.line((260, 1040, 2300, 860), fill=(246, 255, 131, 150), width=8)
    ending.convert("RGB").save(CUTIN_DIR / "ending_solar_route.jpg", "JPEG", quality=96, subsampling=0, optimize=True)

    boss = Image.new("RGBA", (1024, 512), (12, 8, 14, 255))
    draw = ImageDraw.Draw(boss, "RGBA")
    for radius, alpha in ((430, 60), (260, 96), (130, 150)):
        add_blurred_ellipse(boss, (780 - radius, 250 - radius, 780 + radius, 250 + radius), (255, 176, 70, alpha), radius * 0.18)
    draw.rectangle((0, 0, 1024, 512), outline=(255, 155, 122, 180), width=8)
    draw.polygon([(120, 360), (260, 100), (420, 360)], fill=(255, 155, 122, 42), outline=(255, 210, 130, 140))
    draw.line((64, 404, 960, 120), fill=(255, 222, 118, 85), width=5)
    draw.line((64, 430, 960, 190), fill=(255, 128, 116, 65), width=4)
    boss = boss.filter(ImageFilter.UnsharpMask(radius=1.4, percent=140, threshold=4))
    boss.save(CUTIN_DIR / "solar_gatekeeper_cutin.png")


def make_ui_atlas() -> None:
    atlas = Image.new("RGBA", (512, 256), (0, 0, 0, 0))
    draw = ImageDraw.Draw(atlas, "RGBA")
    fills = [(12, 36, 48), (42, 46, 30), (18, 47, 36), (48, 28, 50)]
    accents = [(90, 190, 245), (246, 202, 90), (120, 235, 145), (220, 126, 235)]
    for tile, (fill, accent) in enumerate(zip(fills, accents)):
        tx = (tile % 2) * 256
        ty = (tile // 2) * 128
        panel = (tx + 8, ty + 8, tx + 248, ty + 120)
        shadow = (panel[0] + 4, panel[1] + 6, panel[2] + 4, panel[3] + 6)
        draw.rounded_rectangle(shadow, 11, fill=(0, 0, 0, 160))
        draw.rounded_rectangle(panel, 10, fill=(fill[0], fill[1], fill[2], 236), outline=(accent[0], accent[1], accent[2], 210), width=3)
        draw.rounded_rectangle((tx + 16, ty + 17, tx + 240, ty + 34), 8, fill=(255, 255, 255, 28))
        draw.rounded_rectangle((tx + 24, ty + 44, tx + 232, ty + 106), 6, fill=(4, 14, 22, 102), outline=(accent[0], accent[1], accent[2], 60), width=2)
        for i in range(9):
            x = tx + 28 + i * 22
            draw.line((x, ty + 48, x + 18, ty + 102), fill=(255, 255, 255, 10), width=1)
        draw.line((tx + 28, ty + 100, tx + 228, ty + 100), fill=(accent[0], accent[1], accent[2], 82), width=3)
    atlas.save(UI_DIR / "pawline_ui_atlas.png")


def draw_vfx_sheet(path: Path, columns: int, rows: int, fw: int, fh: int, palette: tuple[tuple[int, int, int], ...], style: str) -> None:
    rng = random.Random(hash((path.name, style)) & 0xFFFF_FFFF)
    sheet = Image.new("RGBA", (columns * fw, rows * fh), (0, 0, 0, 0))
    for frame in range(columns * rows):
        x0 = (frame % columns) * fw
        y0 = (frame // columns) * fh
        t = frame / max(1, columns * rows - 1)
        cell = Image.new("RGBA", (fw, fh), (0, 0, 0, 0))
        glow = Image.new("RGBA", (fw, fh), (0, 0, 0, 0))
        d = ImageDraw.Draw(cell, "RGBA")
        gd = ImageDraw.Draw(glow, "RGBA")
        color = palette[frame % len(palette)]
        cx, cy = fw * 0.5, fh * 0.52
        radius = (0.18 + 0.52 * math.sin(t * math.pi)) * min(fw, fh)
        alpha = int(220 * (1.0 - t * 0.42))

        if style in {"slash", "smear", "wind"}:
            for i in range(5):
                off = (i - 2) * min(fw, fh) * 0.05
                d.arc((cx - radius * 1.6, cy - radius * 0.7 + off, cx + radius * 1.6, cy + radius * 0.7 + off), 205, 338,
                      fill=(color[0], color[1], color[2], max(30, alpha - i * 30)), width=max(2, int(fw * 0.035)))
            gd.ellipse((cx - radius, cy - radius * 0.55, cx + radius, cy + radius * 0.55), fill=(color[0], color[1], color[2], 46))
        elif style in {"thunder", "holy", "crystal"}:
            points = [(cx, cy - radius)]
            for i in range(1, 6):
                yy = cy - radius + i * radius * 2 / 6
                xx = cx + rng.uniform(-0.26, 0.26) * fw
                points.append((xx, yy))
            d.line(points, fill=(255, 255, 255, alpha), width=max(2, int(fw * 0.05)))
            d.line(points, fill=(color[0], color[1], color[2], max(80, alpha)), width=max(1, int(fw * 0.025)))
            gd.ellipse((cx - radius * 0.7, cy - radius * 0.7, cx + radius * 0.7, cy + radius * 0.7), fill=(color[0], color[1], color[2], 56))
        elif style in {"smoke", "earth", "wood"}:
            for _ in range(14):
                rr = rng.uniform(0.08, 0.22) * min(fw, fh) * (1.0 + t)
                px = cx + rng.uniform(-radius, radius)
                py = cy + rng.uniform(-radius * 0.45, radius * 0.45) - t * fh * 0.14
                d.ellipse((px - rr, py - rr, px + rr, py + rr), fill=(color[0], color[1], color[2], int(alpha * rng.uniform(0.18, 0.48))))
        else:
            for ring in range(4):
                r = radius * (0.55 + ring * 0.24)
                d.ellipse((cx - r, cy - r, cx + r, cy + r), outline=(color[0], color[1], color[2], max(40, alpha - ring * 38)), width=max(2, int(min(fw, fh) * 0.025)))
            for _ in range(18):
                angle = rng.random() * math.tau
                dist = rng.uniform(radius * 0.2, radius * 1.25)
                px = cx + math.cos(angle) * dist
                py = cy + math.sin(angle) * dist
                rr = rng.uniform(1.2, max(1.3, min(fw, fh) * 0.035))
                d.ellipse((px - rr, py - rr, px + rr, py + rr), fill=(255, 255, 255, int(alpha * 0.55)))
            gd.ellipse((cx - radius, cy - radius, cx + radius, cy + radius), fill=(color[0], color[1], color[2], 64))

        glow = glow.filter(ImageFilter.GaussianBlur(max(1.0, min(fw, fh) * 0.055)))
        glow.alpha_composite(cell)
        sheet.alpha_composite(glow, (x0, y0))
    sheet.save(path)


def make_vfx_sheets() -> None:
    specs = [
        ("slash_effect_sheet.png", 1, 8, 64, 64, ((120, 210, 255), (255, 255, 255)), "slash"),
        ("slash_gameboy_effect_sheet.png", 1, 8, 64, 64, ((255, 150, 170), (255, 235, 210)), "slash"),
        ("heal_effect_sheet.png", 4, 4, 128, 128, ((120, 255, 165), (220, 255, 170)), "holy"),
        ("heal_gameboy_effect_sheet.png", 4, 4, 128, 128, ((175, 255, 190), (240, 255, 190)), "holy"),
        ("fire_explosion_sheet.png", 4, 4, 64, 64, ((255, 110, 52), (255, 224, 96)), "burst"),
        ("ice_burst_sheet.png", 5, 4, 192, 192, ((125, 220, 255), (230, 255, 255)), "crystal"),
        ("thunder_strike_sheet.png", 13, 1, 64, 64, ((185, 135, 255), (255, 255, 130)), "thunder"),
        ("water_impact_sheet.png", 4, 4, 64, 64, ((80, 190, 255), (190, 250, 255)), "burst"),
        ("water_ball_impact_sheet.png", 4, 4, 64, 64, ((90, 210, 255), (180, 255, 255)), "burst"),
        ("dark_impact_sheet.png", 16, 1, 48, 64, ((130, 90, 210), (245, 160, 255)), "burst"),
        ("acid_impact_sheet.png", 16, 1, 32, 32, ((150, 255, 90), (255, 255, 110)), "burst"),
        ("earth_impact_sheet.png", 7, 1, 48, 48, ((190, 130, 80), (240, 210, 120)), "earth"),
        ("smoke_puff_sheet.png", 13, 1, 64, 64, ((150, 160, 170), (210, 220, 230)), "smoke"),
        ("holy_flash_sheet.png", 16, 1, 48, 48, ((255, 246, 150), (255, 255, 255)), "holy"),
        ("wind_breath_sheet.png", 18, 1, 32, 32, ((180, 255, 220), (230, 255, 255)), "wind"),
        ("wind_hit_sheet.png", 3, 1, 32, 64, ((150, 255, 230), (255, 255, 255)), "wind"),
        ("wood_hit_sheet.png", 7, 1, 32, 32, ((130, 230, 120), (230, 190, 90)), "wood"),
        ("hit_flash_sheet.png", 7, 1, 48, 48, ((255, 255, 255), (255, 210, 140)), "burst"),
        ("smear_horizontal_sheet.png", 5, 1, 48, 48, ((255, 255, 255), (120, 220, 255)), "smear"),
        ("thrust_sheet.png", 1, 3, 64, 64, ((125, 220, 255), (255, 255, 255)), "slash"),
        ("explosion_large_sheet.png", 18, 1, 48, 48, ((255, 90, 50), (255, 240, 120)), "burst"),
        ("fire_breath_sheet.png", 8, 3, 48, 48, ((255, 100, 60), (255, 220, 90)), "burst"),
        ("magic_mirror_sheet.png", 1, 5, 128, 128, ((170, 150, 255), (230, 255, 255)), "crystal"),
        ("energy_impact_sheet.png", 1, 8, 128, 128, ((120, 190, 255), (255, 255, 255)), "burst"),
        ("crystal_sheet.png", 6, 1, 128, 128, ((150, 220, 255), (245, 245, 255)), "crystal"),
        ("air_burst_sheet.png", 3, 3, 48, 48, ((190, 255, 235), (255, 255, 255)), "wind"),
        ("thunder_splash_sheet.png", 14, 1, 48, 48, ((190, 150, 255), (255, 255, 120)), "thunder"),
        ("smoke_dust_sheet.png", 15, 1, 48, 64, ((145, 135, 118), (220, 210, 188)), "smoke"),
    ]
    for filename, columns, rows, fw, fh, palette, style in specs:
        draw_vfx_sheet(VFX_DIR / filename, columns, rows, fw, fh, palette, style)


def sine_mix(t: float, freqs: tuple[float, ...], weights: tuple[float, ...]) -> float:
    return sum(math.sin(t * f * math.tau) * w for f, w in zip(freqs, weights))


def write_wav(path: Path, duration: float, stage: int, sr: int = 44100) -> None:
    rng = random.Random(22000 + stage * 113)
    roots = [110.0, 123.47, 130.81, 146.83, 98.0, 103.83, 92.5, 87.31, 82.41, 164.81]
    root = roots[stage % len(roots)]
    scale = [1.0, 1.125, 1.25, 1.5, 1.6875, 2.0]
    total = int(duration * sr)
    samples = bytearray()
    chime_times = [rng.uniform(1.0, duration - 2.0) for _ in range(34)]
    chime_notes = [root * rng.choice(scale) * rng.choice((1.0, 2.0, 4.0)) for _ in chime_times]

    for n in range(total):
        t = n / sr
        fade = min(1.0, t / 2.5, (duration - t) / 2.5)
        pad = sine_mix(t, (root * 0.5, root, root * 1.5, root * 2.0), (0.18, 0.11, 0.07, 0.035))
        slow = math.sin(t * 0.055 * math.tau + stage) * 0.16
        pulse = math.sin(t * (0.16 + stage * 0.008) * math.tau) * 0.05
        arp = 0.0
        beat = int(t * (1.6 + stage * 0.025)) % 6
        note = root * scale[beat] * 2.0
        phase = (t * (1.6 + stage * 0.025)) % 1.0
        arp_env = math.exp(-phase * 5.2)
        arp += math.sin(t * note * math.tau) * arp_env * 0.045
        chime = 0.0
        for ct, cn in zip(chime_times, chime_notes):
            local = t - ct
            if 0.0 <= local <= 1.8:
                env = math.exp(-local * 2.8) * math.sin(min(1.0, local * 12.0) * math.pi * 0.5)
                chime += math.sin(t * cn * math.tau) * env * 0.050
                chime += math.sin(t * cn * 2.01 * math.tau) * env * 0.018
        value = (pad + arp + chime + slow + pulse) * fade * 0.46
        pan = math.sin(t * 0.071 * math.tau + stage * 0.7) * 0.24
        left = max(-1.0, min(1.0, value * (1.0 - pan)))
        right = max(-1.0, min(1.0, value * (1.0 + pan)))
        samples += struct.pack("<hh", int(left * 32767), int(right * 32767))

    with wave.open(str(path), "wb") as wav:
        wav.setnchannels(2)
        wav.setsampwidth(2)
        wav.setframerate(sr)
        wav.writeframes(samples)


def make_music() -> None:
    for stage, name in enumerate(STAGE_NAMES):
        write_wav(MUSIC_DIR / f"stage_{name}.wav", 48.0, stage)
    write_wav(MUSIC_DIR / "outer_space_loop.wav", 62.0, 8)
    write_wav(MUSIC_DIR / "layer_danger.wav", 28.0, 9)
    write_wav(MUSIC_DIR / "result_victory.wav", 32.0, 5)
    write_wav(MUSIC_DIR / "result_defeat.wav", 28.0, 1)


def main() -> None:
    ensure_dirs()
    for stage in range(10):
        make_space_background(stage, BACKGROUND_DIR / f"stage_{stage:02d}_space_hd.jpg")
    make_cutin_art()
    make_ui_atlas()
    make_vfx_sheets()
    make_music()


if __name__ == "__main__":
    main()
