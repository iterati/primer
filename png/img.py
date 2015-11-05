from PIL import Image, ImageDraw, ImageFont
import random

font = ImageFont.truetype("/usr/share/fonts/OTF/Inconsolata.otf", 10)

# 10, 40, 30, //Silver
# 10, 65, 55, //Luna

colors = [
  (  0,   0,   0),
  (255, 255, 255),
  ( 64,   0,   0),
  ( 64,  64,   0),
  (  0,  64,   0),
  (  0,  64,  64),
  (  0,   0,  64),
  ( 64,   0,  64),
  (255,   0,   0),
  (224,  32,   0),
  (192,  64,   0),
  (160,  96,   0),
  (128, 128,   0),
  ( 96, 160,   0),
  ( 64, 192,   0),
  ( 32, 224,   0),
  (  0, 255,   0),
  (  0, 224,  32),
  (  0, 192,  64),
  (  0, 160,  96),
  (  0, 128, 128),
  (  0,  96, 160),
  (  0,  64, 192),
  (  0,  32, 224),
  (  0,   0, 255),
  ( 32,   0, 224),
  ( 64,   0, 192),
  ( 96,   0, 160),
  (128,   0, 128),
  (160,   0,  96),
  (192,   0,  64),
  (224,   0,  32),
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
    grid = Image.new("RGB", (64 * 8, 80 * 4), (0, 0, 0))
    for i in range(4):
        strip = make_strip(8 * i, colors[8 * i:8 * (i + 1)])
        grid.paste(strip, (0, 80 * i))

    return grid


def unpack(c):
    s = c >> 6
    r, g, b = colors[c & 31]
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

        if tick < c_time / 2:
            yield make_pixel(*morph_color(tick, c_time / 2, 0, colors[cur_color]))
        elif tick < c_time:
            yield make_pixel(*morph_color(tick - (c_time / 2), c_time - (c_time / 2), colors[cur_color], 0))
        else:
            yield make_pixel(0, 0, 0)

        tick += 1


def make_morph(c_time, b_time, strobes,  colors):
    c0 = cur_color = tick = 0
    while True:
        if tick >= c_time + b_time:
            tick = 0
            c0 += 1
            if c0 >= strobes:
                c0 = 0
                cur_color = (cur_color + 1) % len(colors)

        if tick < c_time:
            yield make_pixel(*morph_color(
                tick + (c_time + b_time) * c0, (c_time + b_time) * strobes,
                colors[cur_color],
                colors[(cur_color + 1) % len(colors)]
            ))
        else:
            yield make_pixel(0, 0, 0)

        tick += 1


def make_dashdop(c_time, d_time, b_time, dops, colors):
    tick = 0
    while True:
        if tick >= ((len(colors) - 1) * c_time) + ((d_time + b_time) * dops) + b_time:
            tick = 0

        if tick < (len(colors) - 1) * c_time:
            yield make_pixel(*unpack(colors[(tick / c_time) + 1]))
        else:
            t0 = tick - ((len(colors) - 1) * c_time)
            if t0 % (b_time + d_time) >= b_time:
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
    ts = {0: 4, 1: 16, 2: 32}
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


def make_chase(c_time, b_time, steps, colors):
    c0 = c1 = cur_color = tick = 0
    while True:
        if tick >= c_time + b_time:
            tick = 0
            c0 += 1
            if c0 >= (steps - 1):
                c0 = 0
                cur_color = (cur_color + 1) % len(colors)

        if tick < c_time:
            c1 = tick / (c_time / steps)
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


def make_comet(c_time, b_time, step, colors):
    c0 = c1 = cur_color = tick = 0
    while True:
        if tick >= c_time + b_time:
            cur_color = (cur_color + 1) % len(colors)
            tick = 0
            c0 += step if c1 == 0 else -1 * step
            if c0 <= 0:
                c1 = 0
            elif c0 >= c_time:
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
        'Strobe': make_strobe(10, 15, colors),
        'Hyper': make_strobe(25, 25, colors),
        'Dops': make_strobe(3, 22, colors),
        'Strobie': make_strobe(6, 44, colors),
        'Pulse': make_pulse(200, 50, colors),
        'Seizure': make_strobe(10, 190, colors),
        'Tracer': make_tracer(6, 44, colors),
        'Dashdops': make_dashdop(3, 3, 20, 2, colors),
        'Blinke': make_blinke(10, 100, colors),
        'Edge': make_edge(4, 16, 40, colors),
        'Lego': make_lego(16, colors),
        'Chase': make_chase(100, 20, 5, colors),
        'Morph': make_morph(34, 34, 4, colors),
        'Ribbon': make_strobe(22, 0, colors),
        'Comet': make_comet(30, 16, 2, colors),
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
        ('Strobe',   [0x08, 0x10, 0x18]),
        ('Hyper',    [0x08, 0x10, 0x18]),
        ('Dops',     [0x08, 0x10, 0x18]),
        ('Strobie',  [0x08, 0x10, 0x18]),
        ('Pulse',    [0x08, 0x10, 0x18]),
        ('Seizure',  [0x08, 0x10, 0x18]),
        ('Tracer',   [0x08, 0x10, 0x18]),
        ('Dashdops', [0x08, 0x10, 0x18]),
        ('Blinke',   [0x08, 0x10, 0x18]),
        ('Edge',     [0x08, 0x10, 0x18]),
        ('Lego',     [0x08, 0x10, 0x18]),
        ('Chase',    [0x08, 0x10, 0x18]),
        ('Morph',    [0x08, 0x10, 0x18]),
        ('Ribbon',   [0x08, 0x10, 0x18]),
        ('Comet',    [0x08, 0x10, 0x18]),
        ('Candy',    [0x08, 0x10, 0x18]),
    ]

    for i, (name, colors) in enumerate(anims):
        p = make_anim(name, ticks, colors)
        p.save("anim%02d.png" % (i + 1))


def make_modes(ticks):
    anims = [
        ['Speed - Low',
         ('Dops', [0x01, 0x08, 0x01, 0x0C, 0x01, 0x10, 0x01, 0x14, 0x01, 0x18, 0x01, 0x1C]),
         ('Dashdops', [0x01, 0x1E, 0x1C, 0x1A, 0x18, 0x16, 0x14, 0x12, 0x10, 0x0E, 0x0C, 0x0A, 0x08]),
         ],
        ['Speed - Medium',
         ('Blinke', [0x01, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02]),
         ('Strobie', [0x0A, 0x0D, 0x10, 0x13, 0x16, 0x19, 0x1C, 0x1F]),
         ],
        ['Speed - High',
         ('Hyper', [0x1E, 0x0E, 0x16]),
         ('Strobe', [0x00]),
         ],
        ['Tilt X - Low',
         ('Edge', [0x1E, 0x16, 0x19, 0x88, 0x9F, 0x9E, 0x9D, 0x9C, 0xDB, 0xDA, 0xDA, 0xDA]),
         ('Edge', [0x16, 0x5E, 0x5B, 0x94, 0x95, 0x96, 0x97, 0x98, 0xD9, 0xDA, 0xDA, 0xDA]),
         ],
        ['Tilt X - Medium',
         ('Tracer', [0x02, 0x08, 0x0B, 0x0E, 0x11, 0x14, 0x17, 0x1A, 0x1D]),
         ('Tracer', [0x06, 0x08, 0x0B, 0x0E, 0x11, 0x14, 0x17, 0x1A, 0x1D]),
         ],
        ['Tilt X - High',
         ('Candy', [0x08, 0x0B, 0x08, 0x1D, 0x08, 0x0E, 0x08, 0x1A]),
         ('Candy', [0x18, 0x1B, 0x18, 0x15, 0x18, 0x1E, 0x18, 0x12]),
         ],
        ['Tilt Y - Low',
         ('Chase', [0x1A, 0x1D, 0x08, 0x0B, 0x0E]),
         ('Chase', [0x12, 0x15, 0x18, 0x1B, 0x1E]),
         ],
        ['Tilt Y - Medium',
         ('Ribbon', [0x08, 0x0A, 0x00, 0x00, 0x10, 0x12, 0x00, 0x00, 0x18, 0x1A, 0x00, 0x00]),
         ('Ribbon', [0x08, 0x00, 0x0C, 0x00, 0x10, 0x00, 0x14, 0x00, 0x18, 0x00, 0x1C, 0x00]),
         ],
        ['Tilt Y - High',
         ('Comet', [0x1E, 0x1F, 0x08, 0x09, 0x0A]),
         ('Comet', [0x16, 0x17, 0x18, 0x19, 0x1A]),
         ],
        ['Flip Z - Low',
         ('Pulse', [0x16, 0x00, 0x18, 0x00, 0x1A, 0x00]),
         ('Pulse', [0x1E, 0x00, 0x08, 0x00, 0x0A, 0x00]),
         ],
        ['Flip Z - Medium',
         ('Lego', [0x08, 0x0A, 0x0C, 0x0D]),
         ('Lego', [0x18, 0x1A, 0x1C, 0x1D]),
         ],
        ['Flip Z - High',
         ('Morph', [0x10, 0x18, 0x08, 0x18]),
         ('Morph', [0x10, 0x08, 0x18, 0x08]),
         ],
        ['Off',
         ('Strobie', [0x08, 0xD8]),
         ('Strobe', [0x00]),
         ],
        ['Off',
         ('Strobie', [0x18, 0xC8]),
         ('Strobe', [0x00]),
         ],
        ['Off',
         ('Strobie', [0x1A, 0xCE]),
         ('Strobe', [0x00]),
         ],
        ['Off',
         ('Strobie', [0x0E, 0xDA]),
         ('Strobe', [0x00]),
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
