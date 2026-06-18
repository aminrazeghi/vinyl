#!/usr/bin/env python3
"""Generates the Groovr app icon: a vinyl record with a cyan label."""
import math
from PIL import Image, ImageDraw
import os

SIZE = 512
cx = cy = SIZE // 2

img = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
draw = ImageDraw.Draw(img)

def circle(draw, cx, cy, r, fill=None, outline=None, width=1):
    draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=fill, outline=outline, width=width)

# ── Outer record body ────────────────────────────────────────────────────────
circle(draw, cx, cy, 248, fill=(18, 18, 24))

# Groove rings — subtle dark lines to suggest vinyl grooves
for r in range(220, 90, -8):
    alpha = 55 if r % 16 == 0 else 30
    circle(draw, cx, cy, r, outline=(255, 255, 255, alpha), width=1)

# ── Label area ───────────────────────────────────────────────────────────────
LABEL_R = 88
circle(draw, cx, cy, LABEL_R, fill=(10, 10, 20))

# Cyan ring around label
circle(draw, cx, cy, LABEL_R, outline=(0, 210, 210), width=3)

# Label gradient feel — concentric rings from dark to slightly lighter
for i, r in enumerate(range(LABEL_R - 4, 20, -8)):
    t = i / ((LABEL_R - 4) / 8)
    lum = int(15 + t * 20)
    circle(draw, cx, cy, r, fill=(lum, lum, lum + 8))

# ── Vinyl record wave lines on the label (decorative) ────────────────────────
for angle_deg in range(0, 360, 45):
    angle = math.radians(angle_deg)
    x1 = cx + int(28 * math.cos(angle))
    y1 = cy + int(28 * math.sin(angle))
    x2 = cx + int(60 * math.cos(angle))
    y2 = cy + int(60 * math.sin(angle))
    draw.line([(x1, y1), (x2, y2)], fill=(0, 210, 210, 140), width=2)

# ── Spindle hole ─────────────────────────────────────────────────────────────
circle(draw, cx, cy, 12, fill=(0, 0, 0, 0))   # transparent hole
circle(draw, cx, cy, 12, outline=(0, 210, 210), width=2)

# ── Thin cyan edge ring ───────────────────────────────────────────────────────
circle(draw, cx, cy, 247, outline=(0, 210, 210, 80), width=2)

out = os.path.join(os.path.dirname(__file__), "groovr_icon.png")
# Ensure exactly 512×512 regardless of display DPI scaling
if img.size != (512, 512):
    img = img.resize((512, 512), Image.LANCZOS)
img.save(out, dpi=(96, 96))
print(f"Saved {out} ({img.size[0]}x{img.size[1]})")
