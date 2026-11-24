from PIL import Image
import numpy as np

from pathlib import Path

folder = Path("./")

for file in folder.glob("*.png"):
    if file.is_file():
        file.unlink()

img = Image.open("../assets/Bugs.png").convert("RGBA")
a = np.array(img)

# Treat the top-left pixel as the background color (or transparent)
bg = a[0, 0]
mask = (a != bg).any(axis=-1)
if bg[3] == 0:
    mask = a[..., 3] > 0

def runs(profile, min_gap=1):
    """(start, end) spans of True values; gaps shorter than min_gap are bridged."""
    spans, start, gap = [], None, 0
    for i, v in enumerate(profile):
        if v:
            if start is None:
                start = i
            gap = 0
            end = i + 1
        elif start is not None:
            gap += 1
            if gap >= min_gap:
                spans.append((start, end))
                start = None
    if start is not None:
        spans.append((start, end))
    return spans

n = 0
for y0, y1 in runs(mask.any(axis=1)):
    for x0, x1 in runs(mask[y0:y1].any(axis=0)):
        img.crop((x0, y0, x1, y1)).save(f"bug_{n:02d}.png")
        print(f"bug_{n:02d}: x={x0} y={y0} w={x1-x0} h={y1-y0}")
        n += 1

print("total:", n)