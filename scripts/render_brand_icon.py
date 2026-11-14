#!/usr/bin/env python3
"""Render Nexvary's original eye-seal vectors and size-specific Windows icon.

Run locally with fontTools, Pillow, librsvg and Cairo. Production builds consume
the committed SVG/ICO assets and do not need these design-time dependencies.
Letter outlines use the bundled OFL Noto font; no runtime font dependency.
"""
from pathlib import Path
from tempfile import TemporaryDirectory
import math
import ctypes as C
import ctypes.util
from fontTools.ttLib import TTFont
from fontTools.pens.svgPathPen import SVGPathPen
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
FONT = TTFont(ROOT / 'assets/fonts/NotoKufiArabic.ttf')
GLYPHS = FONT.getGlyphSet()
CMAP = FONT.getBestCmap()
UNITS = FONT['head'].unitsPerEm


def letters(text, size, y, curved=False):
    scale = size / UNITS
    advances = [FONT['hmtx'][CMAP[ord(c)]][0] * scale + 2 for c in text]
    total = sum(advances)
    cursor = -total / 2
    paths = []
    for char, advance in zip(text, advances):
        pen = SVGPathPen(GLYPHS)
        GLYPHS[CMAP[ord(char)]].draw(pen)
        if curved:
            angle = (cursor + advance / 2) / 190
            x, baseline = 256 + 190 * math.sin(angle), 256 - 190 * math.cos(angle)
            transform = f'translate({x:.3f} {baseline:.3f}) rotate({math.degrees(angle):.3f}) translate({-advance/2:.3f} 0) scale({scale:.6f} {-scale:.6f})'
        else:
            transform = f'translate({256+cursor:.3f} {y}) scale({scale:.6f} {-scale:.6f})'
        paths.append(f'<path d="{pen.getCommands()}" transform="{transform}"/>')
        cursor += advance
    return '<g fill="#edf5fa">' + ''.join(paths) + '</g>'


def vector(compact=False):
    eye = ('M64 256 Q256 66 448 256 Q256 446 64 256Z' if compact
           else 'M94 266 Q256 104 418 266 Q256 428 94 266Z')
    cy, radius = (256, 86) if compact else (266, 66)
    return f'''<svg xmlns="http://www.w3.org/2000/svg" width="512" height="512" viewBox="0 0 512 512">
<title>NEXVARY MR — Operations &amp; Intelligence System</title>
<desc>Original silver eye seal with a blue intelligence iris. Simplified symbol for small Windows icons.</desc>
<defs>
 <linearGradient id="metal" x1="0" y1="0" x2="0.8" y2="1"><stop stop-color="#f5fbff"/><stop offset=".22" stop-color="#8393a4"/><stop offset=".48" stop-color="#f9fcff"/><stop offset=".7" stop-color="#8a99aa"/><stop offset="1" stop-color="#e4edf4"/></linearGradient>
 <radialGradient id="black"><stop stop-color="#152c3b"/><stop offset="1" stop-color="#060b10"/></radialGradient>
 <radialGradient id="iris"><stop stop-color="#031b33"/><stop offset=".5" stop-color="#036fa7"/><stop offset=".85" stop-color="#00bfff"/><stop offset="1" stop-color="#0066a3"/></radialGradient>
 <linearGradient id="eye" x2="0" y2="1"><stop stop-color="#f0f8ff"/><stop offset=".48" stop-color="#a1b5c5"/><stop offset="1" stop-color="#344e62"/></linearGradient>
 <clipPath id="eyeClip"><path d="{eye}"/></clipPath>
</defs>
<circle cx="256" cy="256" r="248" fill="url(#metal)"/>
<circle cx="256" cy="256" r="236" fill="url(#black)" stroke="#e1eaf0" stroke-width="3"/>
<circle cx="256" cy="256" r="225" fill="none" stroke="#00bfff" stroke-width="3"/>
<circle cx="256" cy="256" r="{'211' if compact else '165'}" fill="#07121c" stroke="url(#metal)" stroke-width="{'5' if compact else '4'}"/>
<path d="{eye}" fill="url(#eye)" stroke="#06111a" stroke-width="20" stroke-linejoin="round"/>
<path d="{eye}" fill="url(#eye)" stroke="url(#metal)" stroke-width="10" stroke-linejoin="round"/>
<g clip-path="url(#eyeClip)">
 <circle cx="256" cy="{cy}" r="{radius}" fill="url(#iris)" stroke="#b9f0ff" stroke-width="5"/>
 <path d="M256 {cy-radius+8} V{cy+radius-8} M{256-radius+8} {cy} H{256+radius-8} M{256-radius*.62} {cy-radius*.62} L{256+radius*.62} {cy+radius*.62} M{256-radius*.62} {cy+radius*.62} L{256+radius*.62} {cy-radius*.62}" stroke="#6ce1ff" stroke-width="3" opacity=".8"/>
 <circle cx="256" cy="{cy}" r="{radius*.36}" fill="#02070c" stroke="#00d9ff" stroke-width="5"/>
 <circle cx="{256-radius*.22}" cy="{cy-radius*.27}" r="{radius*.14}" fill="#f3fdff"/>
 <circle cx="{256-radius*.6}" cy="{cy+radius*.25}" r="6" fill="#39ff14"/>
 <circle cx="{256+radius*.63}" cy="{cy-radius*.2}" r="6" fill="#e7faff"/>
</g>
<circle cx="38" cy="256" r="7" fill="#00bfff"/>
<circle cx="474" cy="256" r="7" fill="#39ff14"/>
{'' if compact else letters('NEXVARY MR', 46, 0, True)}
{'' if compact else letters('OPERATIONS', 20, 433) + letters('INTELLIGENCE SYSTEM', 17, 458)}
</svg>'''


def render(source, destination, size):
    rsvg = C.CDLL(ctypes.util.find_library('rsvg-2'))
    cairo = C.CDLL(ctypes.util.find_library('cairo'))
    objects = C.CDLL(ctypes.util.find_library('gobject-2.0'))
    rsvg.rsvg_handle_new_from_data.argtypes = [C.c_char_p, C.c_size_t, C.c_void_p]
    rsvg.rsvg_handle_new_from_data.restype = C.c_void_p
    rsvg.rsvg_handle_render_cairo.argtypes = [C.c_void_p, C.c_void_p]
    rsvg.rsvg_handle_render_cairo.restype = C.c_int
    cairo.cairo_image_surface_create.argtypes = [C.c_int, C.c_int, C.c_int]
    cairo.cairo_image_surface_create.restype = C.c_void_p
    cairo.cairo_create.argtypes = [C.c_void_p]
    cairo.cairo_create.restype = C.c_void_p
    cairo.cairo_scale.argtypes = [C.c_void_p, C.c_double, C.c_double]
    cairo.cairo_surface_write_to_png.argtypes = [C.c_void_p, C.c_char_p]
    cairo.cairo_surface_write_to_png.restype = C.c_int
    cairo.cairo_destroy.argtypes = [C.c_void_p]
    cairo.cairo_surface_destroy.argtypes = [C.c_void_p]
    objects.g_object_unref.argtypes = [C.c_void_p]
    data = Path(source).read_bytes()
    handle = rsvg.rsvg_handle_new_from_data(data, len(data), None)
    if not handle:
        raise RuntimeError('SVG parse failed')
    surface = cairo.cairo_image_surface_create(0, size, size)
    context = cairo.cairo_create(surface)
    try:
        cairo.cairo_scale(context, size / 512, size / 512)
        if not rsvg.rsvg_handle_render_cairo(handle, context):
            raise RuntimeError('SVG rendering failed')
        if cairo.cairo_surface_write_to_png(surface, str(destination).encode()):
            raise RuntimeError('PNG write failed')
    finally:
        cairo.cairo_destroy(context)
        cairo.cairo_surface_destroy(surface)
        objects.g_object_unref(handle)


def main():
    folder = ROOT / 'assets/branding'
    master, symbol = folder / 'nexvary-eye-seal.svg', folder / 'nexvary-eye-symbol.svg'
    master.write_text(vector(), encoding='utf-8')
    symbol.write_text(vector(True), encoding='utf-8')
    sizes = (16, 24, 32, 48, 64, 128, 256)
    with TemporaryDirectory(prefix='nexvary-vector-icons-') as temp:
        frames = []
        for size in sizes:
            png = Path(temp) / f'{size}.png'
            render(symbol if size <= 48 else master, png, size)
            frames.append(Image.open(png).convert('RGBA'))
        frames[-1].save(ROOT / 'packaging/app.ico', format='ICO', append_images=frames[:-1], sizes=[(n,n) for n in sizes])


if __name__ == '__main__':
    main()
