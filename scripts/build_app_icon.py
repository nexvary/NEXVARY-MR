"""Render the original vector master into the multi-resolution ICO.
Maintainer-only dependencies: Pillow and CairoSVG; neither is bundled at runtime.
"""
from pathlib import Path
from io import BytesIO
import cairosvg
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
master=(ROOT/'assets/nexvary-mr.svg').read_bytes()
raster=cairosvg.svg2png(bytestring=master,output_width=1024,output_height=1024)
image=Image.open(BytesIO(raster)).convert('RGBA')
image.resize((256,256),Image.Resampling.LANCZOS).save(ROOT/'assets/nexvary-mr.png')
image.save(ROOT/'packaging/app.ico',format='ICO',sizes=[(n,n) for n in [16,24,32,48,64,128,256]])
print('Rendered one master SVG, PNG compatibility resource and 7-resolution ICO.')
