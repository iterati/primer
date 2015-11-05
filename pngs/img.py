from PIL import Image, ImageDraw, ImageFont
import random

font = ImageFont.truetype("/Library/Fonts/Microsoft/Arial.ttf", 12)

# 10, 40, 30, //Silver
# 10, 65, 55, //Luna

colors = [
    (  0,   0,   0),
    ( 32,  32,  32),
    ( 48,   0,   0),
    ( 40,  40,   0),
    (  0,  48,   0),
    (  0,  40,  40),
    (  0,   0,  48),
    ( 40,   0,  40),

    # red 0x08
    (255,   0,   0),
    (252,  64,   0),
    (248, 128,   0),
    (244, 192,   0),
    (240, 240,   0),
    (192, 244,   0),
    (128, 248,   0),
    ( 64, 252,   0),

    # green 0x10
    (  0, 255,   0),
    (  0, 252,  64),
    (  0, 248, 128),
    (  0, 244, 192),
    (  0, 240, 240),
    (  0, 192, 244),
    (  0, 128, 248),
    (  0,  64, 252),

    # blue 0x18
    (  0,   0, 255),
    ( 64,   0, 252),
    (128,   0, 248),
    (192,   0, 244),
    (240,   0, 240),
    (244,   0, 192),
    (248,   0, 128),
    (252,   0,  64),

    # sat red 0x20
    (240,  32,  32),
    (216,  64,  64),
    (192,  96,  96),
    (160, 128, 128),

    # sat yellow 0x24
    (224, 224,  32),
    (200, 200,  64),
    (176, 176,  96),
    (152, 152, 128),

    # sat green
    ( 32, 240,  32),
    ( 64, 216,  64),
    ( 96, 192,  96),
    (128, 160, 128),

    # sat cyan
    ( 32, 224, 224),
    ( 64, 200, 200),
    ( 96, 176, 176),
    (128, 152, 152),

    # sat blue
    ( 32,  32, 240),
    ( 64,  64, 216),
    ( 96,  96, 192),
    (128, 128, 160),

    # sat magenta
    (224,  32, 224),
    (200,  64, 200),
    (176,  96, 176),
    (152, 128, 152),

    # whites
    (160, 160, 160),
    (192, 128,  64),
    (128, 192,  64),
    ( 64, 192, 128),
    ( 64, 128, 192),
    (128,  64, 192),
    (192,  64, 128),
]


def make_img(i, r, g, b):
    frame = Image.new("RGB", (64, 80), (0, 0, 0))

    color = Image.new("RGB", (31, 31), (r >> 0, g >> 0, b >> 0))
    frame.paste(color, (1, 1))

    color = Image.new("RGB", (31, 31), ((r >> 1, g >> 1, b >> 1)))
    frame.paste(color, (32, 1))

    color = Image.new("RGB", (31, 31), ((r >> 2, g >> 2, b >> 2)))
    frame.paste(color, (1, 32))

    color = Image.new("RGB", (31, 31), ((r >> 3, g >> 3, b >> 3)))
    frame.paste(color, (32, 32))

    draw = ImageDraw.Draw(frame)
    draw.text((11, 66), "%2d 0x%02X" % (i, i), font=font)
    return frame


def make_strip(start, colors):
    strip = Image.new("RGB", (64 * len(colors), 80), (0, 0, 0))
    for i, c in enumerate(colors):
        img = make_img(start + i, *c)
        strip.paste(img, (64 * i, 0))

    return strip


def make_palette(colors):
    strip = Image.new("RGB", (48 * len(colors), 64), (0, 0, 0))
    for i, c in enumerate(colors):
        f = Image.new("RGB", (48, 64), (0, 0, 0))
        p = Image.new("RGB", (46, 46), unpack(c))
        f.paste(p, (1, 1))
        d = ImageDraw.Draw(f)
        d.text((3, 51), "%2d 0x%02X" % (i + 1, c), font=font)

        strip.paste(f, (48 * i, 0))

    return strip


def make_grid(colors):
    grid = Image.new("RGB", (64 * 8, 80 * 8), (0, 0, 0))
    for i in range(8):
        strip = make_strip(8 * i, colors[8 * i:8 * (i + 1)])
        grid.paste(strip, (0, 80 * i))

    return grid


def unpack(c):
    s = c >> 6
    r, g, b = colors[c & 63]
    return r >> s, g >> s, b >> s


def morph_color(t, d, c0, c1):
    r0, g0, b0 = unpack(c0)
    r1, g1, b1 = unpack(c1)
    r = r0 + int((r1 - r0) * (t / float(d)))
    g = g0 + int((g1 - g0) * (t / float(d)))
    b = b0 + int((b1 - b0) * (t / float(d)))
    return r, g, b


def make_pixel(r, g, b):
    return Image.new("RGB", (1, 14), (r, g, b))


def make_strobe(c_time, b_time, colors):
    cur_color = tick = 0
    while True:
        if tick >= c_time + b_time:
            tick = 0
            cur_color = (cur_color + 1) % len(colors)

        if tick < c_time:
            yield make_pixel(*unpack(colors[cur_color]))
        else:
            yield make_pixel(0, 0, 0)

        tick += 1


def make_tracer(c_time, b_time, colors):
    cur_color = tick = 0
    while True:
        if tick >= c_time + b_time:
            tick = 0
            cur_color = (cur_color + 1) % (len(colors) - 1)

        if tick < c_time:
            yield make_pixel(*unpack(colors[(cur_color + 1) % len(colors)]))
        else:
            yield make_pixel(*unpack(colors[0]))

        tick += 1


def make_pulse(c_time, b_time, colors):
    cur_color = tick = 0
    while True:
        if tick >= c_time + b_time:
            tick = 0
            cur_color = (cur_color + 1) % len(colors)

        if tick < c_time:
            yield make_pixel(*morph_color(tick, 200, 0, colors[cur_color]))
        else:
            yield make_pixel(0, 0, 0)

        tick += 1


def make_morph(c_time, b_time, colors):
    c0 = cur_color = tick = 0
    while True:
        if tick >= c_time + b_time:
            tick = 0
            c0 += 1
            if c0 >= 4:
                c0 = 0
                cur_color = (cur_color + 1) % len(colors)

        if tick < c_time:
            yield make_pixel(*morph_color(
                tick + (c_time + b_time) * c0, (c_time + b_time) * 4,
                colors[cur_color],
                colors[(cur_color + 1) % len(colors)]
            ))
        else:
            yield make_pixel(0, 0, 0)

        tick += 1


def make_dashdop(c_time, dops, colors):
    tick = 0
    while True:
        if tick >= ((len(colors) - 1) * c_time) + (23 * dops) + 20:
            tick = 0

        if tick < (len(colors) - 1) * c_time:
            yield make_pixel(*unpack(colors[(tick / c_time) + 1]))
        else:
            t0 = tick - ((len(colors) - 1) * c_time)
            if t0 % 23 >= 20:
                yield make_pixel(*unpack(colors[0]))
            else:
                yield make_pixel(0, 0, 0)

        tick += 1


def make_blinke(c_time, b_time, colors):
    tick = 0
    while True:
        if tick >= (len(colors) * c_time) + b_time:
            tick = 0

        if tick < len(colors) * 10:
            yield make_pixel(*unpack(colors[tick / c_time]))
        else:
            yield make_pixel(0, 0, 0)

        tick += 1


def make_edge(c_time, e_time, b_time, colors):
    tick = 0
    c0 = len(colors) - 1
    while True:
        if tick >= (c0 * c_time * 2) + e_time + b_time:
            tick = 0

        if tick < (len(colors) - 1) * c_time:
            yield make_pixel(*unpack(colors[c0 - (tick / c_time)]))
        elif tick < (c0 * c_time) + e_time:
            yield make_pixel(*unpack(colors[0]))
        elif tick < (c0 * c_time * 2) + e_time:
            yield make_pixel(*unpack(colors[((tick - ((c0 * c_time) + e_time)) / c_time) + 1]))
        else:
            yield make_pixel(0, 0, 0)

        tick += 1


def make_lego(b_time, colors):
    ts = {0: 2, 1: 8, 2: 16}
    cur_color = tick = 0
    c0 = ts[random.randint(0, 2)]
    while True:
        if tick >= c0 + b_time:
            tick = 0
            c0 = ts[random.randint(0, 2)]
            cur_color = (cur_color + 1) % len(colors)

        if tick < c0:
            yield make_pixel(*unpack(colors[cur_color]))
        else:
            yield make_pixel(0, 0, 0)

        tick += 1


def make_chase(colors):
    c0 = c1 = cur_color = tick = 0
    while True:
        if tick >= 120:
            tick = 0
            c0 += 1
            if c0 >= 4:
                c0 = 0
                cur_color = (cur_color + 1) % len(colors)

        if tick < 100:
            c1 = tick / 20
            if c0 == 0:
                yield make_pixel(*unpack(colors[cur_color]))
            else:
                if c1 < c0:
                    yield make_pixel(*unpack(colors[(cur_color + 1) % len(colors)]))
                elif c1 == c0:
                    yield make_pixel(0, 0, 0)
                else:
                    yield make_pixel(*unpack(colors[cur_color]))
        else:
            yield make_pixel(0, 0, 0)

        tick += 1


def make_comet(colors):
    c0 = c1 = cur_color = tick = 0
    while True:
        if tick >= 15 + 8:
            tick = 0
            c0 += 1 if c1 == 0 else -1
            if c0 <= 0:
                c1 = 0
                cur_color = (cur_color + 1) % len(colors)
            elif c0 >= 15:
                c1 = 1

        if tick < c0:
            yield make_pixel(*unpack(colors[cur_color]))
        else:
            yield make_pixel(0, 0, 0)

        tick += 1


def make_candy(c_time, b_time, pick, repeat, colors):
    c0 = c1 = cur_color = tick = 0
    while True:
        if tick >= c_time + b_time:
            tick = 0
            c0 += 1
            if c0 >= pick:
                c0 = 0
                c1 += 1
                if c1 >= repeat:
                    c1 = 0
                    cur_color = (cur_color + 1) % len(colors)

        if tick < c_time:
            yield make_pixel(*unpack(colors[(cur_color + c0) % len(colors)]))
        else:
            yield make_pixel(0, 0, 0)

        tick += 1


def make_anim(name, ticks, colors):
    anims = {
        'Strobe': make_strobe(10, 16, colors),
        'Hyper': make_strobe(34, 34, colors),
        'Dops': make_strobe(3, 20, colors),
        'Strobie': make_strobe(6, 46, colors),
        'Pulse': make_pulse(200, 50, colors),
        'Seizure': make_strobe(10, 190, colors),
        'Tracer': make_tracer(6, 46, colors),
        'Dashdops': make_dashdop(22, 7, colors),
        'Blinke': make_blinke(10, 100, colors),
        'Edge': make_edge(4, 16, 40, colors),
        'Lego': make_lego(8, colors),
        'Chase': make_chase(colors),
        'Morph': make_morph(34, 34, colors),
        'Ribbon': make_strobe(22, 0, colors),
        'Comet': make_comet(colors),
        'Candy': make_candy(10, 16, 3, 2, colors),
    }
    strip = Image.new("RGB", (ticks, 16), (0, 0, 0))
    f = anims[name]
    for i in range(ticks):
        p = f.next()
        strip.paste(p, (i, 1))

    palette = make_palette(colors)

    f = Image.new("RGB", (ticks + 10, 96), (0, 0, 0))
    f.paste(palette, (5, 6))
    f.paste(strip, (5, 75))
    return f


def make_anims(ticks):
    anims = [
        ('Strobe',   [0x09, 0x0B, 0x0D, 0x0F, 0x11, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1D, 0x1F]),
        ('Hyper',    [0x09, 0x0B, 0x0D, 0x00, 0x11, 0x13, 0x15, 0x00, 0x19, 0x1B, 0x1D, 0x00]),
        ('Dops',     [0x09, 0x0B, 0x0D, 0x0F, 0x11, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1D, 0x1F]),
        ('Strobie',  [0x08, 0x0B, 0x0E, 0x11, 0x14, 0x17, 0x1A, 0x1D]),
        ('Pulse',    [0x09, 0x0B, 0x0D, 0x0F, 0x11, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1D, 0x1F]),
        ('Seizure',  [0x09, 0x11, 0x19, 0x0B, 0x13, 0x1B, 0x0D, 0x15, 0x1D, 0x0F, 0x17, 0x1F]),
        ('Tracer',   [0xBD, 0x08, 0x0B, 0x4E, 0x11, 0x14, 0x17, 0x1A, 0x1D]),
        ('Dashdops', [0x38, 0x08, 0x0B, 0x0E, 0x11, 0x14, 0x17, 0x1A, 0x1D]),
        ('Blinke',   [0x38, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02]),
        ('Edge',     [0x08, 0x5F, 0x5E, 0x9D, 0x9C, 0xDB, 0xDA, 0xD9, 0xD8]),
        ('Lego',     [0x09, 0x0D, 0x11, 0x15, 0x19, 0x1D]),
        ('Chase',    [0x09, 0x13, 0x1D, 0x0F, 0x19, 0x0B, 0x15, 0x1F, 0x11, 0x1B, 0x0D, 0x17]),
        ('Morph',    [0x09, 0x0B, 0x0D, 0x0F, 0x11, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1D, 0x1F]),
        ('Ribbon',   [0x20, 0x21, 0x22, 0x23, 0x00, 0x28, 0x29, 0x2A, 0x2B, 0x00, 0x30, 0x31, 0x32, 0x33, 0x00]),
        ('Comet',    [0x09, 0x0B, 0x0D, 0x0F, 0x11, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1D, 0x1F]),
        ('Candy',    [0x09, 0x0B, 0x0D, 0x0F, 0x11, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1D, 0x1F]),
    ]

    for i, (name, colors) in enumerate(anims):
        p = make_anim(name, ticks, colors)
        p.save("anim%02d.png" % (i + 1))


def make_modes(ticks):
    anims = [
        [
            'Speed - Low',
            ('Dops', [0x38]),
            ('Dashdops', [0x38, 0x48, 0x4B, 0x4E, 0x51, 0x54, 0x57, 0x5A, 0x5D]),
        ],
        [
            'Speed - Medium',
            ('Blinke', [0x73, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47]),
            ('Strobie', [0x09, 0x0C, 0x0F, 0x12, 0x15, 0x18, 0x1B, 0x1E]),
        ],
        [
            'Speed - High',
            ('Pulse', [0x49, 0x4B, 0x4D, 0x4F, 0x51, 0x53, 0x55, 0x57, 0x59, 0x5B, 0x5D, 0x5F]),
            ('Strobe', [0x00]),
        ],
        [
            'Tilt X - Low',
            ('Tracer', [0xB2, 0x48, 0x4B, 0x4E, 0x51, 0x54, 0x57, 0x5A, 0x5D]),
            ('Tracer', [0xB6, 0x48, 0x4B, 0x4E, 0x51, 0x54, 0x57, 0x5A, 0x5D]),
        ],
        [
            'Tilt X - Medium',
            ('Candy', [0x48, 0x4A, 0x4C, 0x4E, 0x50, 0x52, 0x54, 0x56, 0x58, 0x5A, 0x5C, 0x5E]),
            ('Candy', [0x48, 0x4C, 0x00, 0x4E, 0x52, 0x00, 0x54, 0x58, 0x00, 0x5A, 0x5E, 0x00]),
        ],
        [
            'Tilt X - High',
            ('Edge', [0x08, 0x5F, 0x5E, 0x9D, 0x9C, 0xDB, 0xDA, 0xD9, 0xD8, 0x00, 0x00, 0x00]),
            ('Edge', [0x18, 0x59, 0x5A, 0x9B, 0x9C, 0xDD, 0xDE, 0xDF, 0xC8, 0x00, 0x00, 0x00]),
        ],
        [
            'Tilt Y - Low',
            ('Chase', [0x49, 0x51, 0x59, 0x4B, 0x53, 0x5B, 0x4D, 0x55, 0x5D, 0x4F, 0x57, 0x5F]),
            ('Chase', [0x5F, 0x57, 0x4F, 0x5D, 0x55, 0x4D, 0x5B, 0x53, 0x4B, 0x59, 0x51, 0x49]),
        ],
        [
            'Tilt Y - Medium',
            ('Ribbon', [0x48, 0x61, 0x63, 0x00, 0x50, 0x69, 0x6B, 0x00, 0x58, 0x71, 0x73, 0x00]),
            ('Ribbon', [0x48, 0x48, 0x48, 0x00, 0x50, 0x50, 0x50, 0x00, 0x58, 0x58, 0x58, 0x00]),
        ],
        [
            'Tilt Y - High',
            ('Comet', [0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C]),
            ('Comet', [0x4A, 0x4E, 0x52, 0x56, 0x5A, 0x5E]),
        ],
        [
            'Flip Z - Low',
            ('Pulse', [0x56, 0x00, 0x58, 0x00, 0x5A, 0x00]),
            ('Pulse', [0x5E, 0x00, 0x48, 0x00, 0x4A, 0x00]),
        ],
        [
            'Flip Z - Medium',
            ('Lego', [0x48, 0x4A, 0x4C, 0x4D]),
            ('Lego', [0x58, 0x5A, 0x5C, 0x5D]),
        ],
        [
            'Flip Z - High',
            ('Morph', [0x48, 0x4C, 0x50]),
            ('Morph', [0x50, 0x54, 0x58]),
        ],
    ]

    for i, [mode, anim1, anim2] in enumerate(anims):
        f = Image.new("RGB", (ticks + 10, 192 + 16), (0, 0, 0))
        p1 = make_anim(anim1[0], ticks, anim1[1])
        p2 = make_anim(anim2[0], ticks, anim2[1])
        f.paste(p1, (0, 0 + 16))
        f.paste(p2, (0, 96 + 16))
        d = ImageDraw.Draw(f)
        d.text((5, 3), "%s: %s -> %s" % (mode, anim1[0], anim2[0]), font=font)
        f.save("mode%02d.png" % (i + 1))


make_grid(colors).save("palette.png")
make_anims(768)
make_modes(768)
