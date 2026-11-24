# pip install pillow numpy scipy
from PIL import Image
import numpy as np
from scipy import ndimage

from pathlib import Path

# folder = Path("./")

# for file in folder.glob("*.png"):
#     if file.is_file():
#         file.unlink()

img = Image.open("../assets/images/Timon.png").convert("RGBA")
a = np.array(img)

# "Background" = fully transparent OR pure white (check which your sheet uses)
bg = (a[..., 3] == 0) | ((a[..., :3] == 255).all(axis=-1))
mask = ~bg

# Merge pixels separated by tiny gaps so each asset becomes one blob
mask = ndimage.binary_closing(mask, structure=np.ones((5, 5)))
labels, _ = ndimage.label(mask)

for i, sl in enumerate(ndimage.find_objects(labels)):
    y, x = sl
    w, h = x.stop - x.start, y.stop - y.start
    if w < 8 or h < 8:        # skip specks
        continue
    img.crop((x.start, y.start, x.stop, y.stop)).save(f"timon_{i:02d}.png")
    print(f"asset_{i:02d}: x={x.start} y={y.start} w={w} h={h}")