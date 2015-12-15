#include "pattern.h"

uint8_t _r, _g, _b, r0, g0, b0, r1, g1, b1;


/*******************************************************************************
 ** BASE ANIMATIONS
 ******************************************************************************/
void _strobe(Pattern* p, uint8_t ct, uint8_t bt, uint8_t lt,
             uint8_t repeat, uint8_t pick, uint8_t skip, bool doit) {
  int8_t c;
  uint16_t sT;

  repeat = (repeat == 0) ? 1 : repeat;
  pick = (pick == 0) ? p->num_colors : pick;
  skip = (skip == 0 || skip > pick) ? pick : skip;

  sT = ((ct + bt) * pick) * repeat;
  if (pick == 2 && skip == 1) sT += ct + bt;

  if (p->tick >= sT + lt) {
    p->tick = 0;
    p->cur_color += skip;
    if (p->cur_color >= p->num_colors) {
      if (pick == skip) {
        p->cur_color = 0;
      } else {
        p->cur_color %= p->num_colors;
      }
    }
  }

  if (doit) {
  if (p->tick < sT) {
    if (p->tick % (ct + bt) < ct) {
      c = ((p->tick / (ct + bt)) % pick) + p->cur_color;
      if (pick == skip) {
        if (c >= p->num_colors) {
          _r = _g = _b = 0;
        } else {
          unpackColor(p->colors[c], _r, _g, _b);
        }
      } else {
        unpackColor(p->colors[c % p->num_colors], _r, _g, _b);
      }
    } else {
      _r = _g = _b = 0;
    }
  } else {
    _r = _g = _b = 0;
  }
  }
}

void _tracer(Pattern* p, uint8_t cst, uint8_t cbt, uint8_t tst, uint8_t tbt,
             uint8_t repeat, uint8_t pick, uint8_t skip, uint8_t pad, bool doit) {
  int8_t c;
  uint16_t t, tT, oT;

  pick = (pick == 0) ? (p->num_colors - 1) : pick;
  skip = (skip == 0 || skip > pick) ? pick : skip;

  if (pad == 0) {
    oT = ((cst + cbt) * pick) + cbt;
    if (repeat == 0) {
      tT = tbt + tst;
    } else {
      tT = ((tbt + tst) * repeat) - tbt;
    }
  } else {
    tT = ((tbt + tst) * repeat) + tbt;
    oT = ((cst + cbt) * pick) - cbt;
  }

  if (p->tick >= oT + tT) {
    p->tick = 0;
    p->cur_color += skip;
    if (p->cur_color >= (p->num_colors - 1)) {
      if (pick == skip) {
        p->cur_color = 0;
      } else {
        p->cur_color %= p->num_colors - 1;
      }
    }
  }

  if (doit) {
  if (p->tick < oT) {
    if (pad == 0) {
      if (p->tick % (cst + cbt) < cbt) {
        c = -1;
      } else {
        c = (p->tick / (cbt + cst)) + p->cur_color + 1;
      }
    } else {
      if (p->tick % (cst + cbt) < cst) {
        c = (p->tick / (cbt + cst)) + p->cur_color + 1;
      } else {
        c = -1;
      }
    }
  } else {
    t = p->tick - oT;
    if (pad == 1 || repeat == 0) {
      if (t % (tbt + tst) < tbt) {
        c = -1;
      } else {
        c = 0;
      }
    } else {
      if (t % (tbt + tst) < tst) {
        c = 0;
      } else {
        _r = _g = _b = 0;
      }
    }
  }

  if (c == -1) {
    _r = _g = _b = 0;
  } else if (c == 0) {
    unpackColor(p->colors[0], _r, _g, _b);
  } else {
    if (pick == skip) {
      if (c > p->num_colors) {
        _r = _g = _b = 0;
      } else {
        unpackColor(p->colors[c], _r, _g, _b);
      }
    } else {
      unpackColor(p->colors[(c % (p->num_colors - 1)) + 1], _r, _g, _b);
    }
  }
  }
}

void _edge(Pattern* p, uint8_t cst, uint8_t cbt, uint8_t est, uint8_t ebt,
           uint8_t pick, bool doit) {
  int8_t c;
  uint16_t t, hT;

  pick = (pick == 0) ? p->num_colors : pick;
  hT = (cst + cbt) * (pick - 1);

  if (p->tick >= (hT + hT + est + ebt)) {
    p->tick = 0;
    p->cur_color += pick;
    if (p->cur_color >= p->num_colors) {
      p->cur_color = 0;
    }
  }

  if (doit) {
  if (p->tick < hT) {
    t = p->tick;
    if (t % (cst + cbt) < cst) {
      c = ((pick - 1) - (t / (cst + cbt))) + p->cur_color;
    } else {
      c = -1;
    }
  } else if (p->tick < hT + est) {
    c = p->cur_color;
  } else if (p->tick < hT + hT + est) {
    t = p->tick - (hT + est);
    if (t % (cst + cbt) < cst) {
      c = ((t / (cst + cbt)) + 1) + p->cur_color;
    } else {
      c = -1;
    }
  } else {
    c = -1;
  }

  if (c == -1 || c >= p->num_colors) {
    _r = _g = _b = 0;
  } else {
    unpackColor(p->colors[c], _r, _g, _b);
  }
  }
}

void _flux(Pattern *p, uint8_t ct, uint8_t bt, uint8_t lt, uint8_t dir,
           uint8_t change, uint8_t dynamic, uint8_t target, uint8_t segments, bool doit) {
  int8_t c;
  uint16_t t, cT, bT, sT;
  uint8_t segs;

  if (target == 0) {
    cT = ct;
    if (dir == 0) {
      segs = segments;
      bT = bt * (segments - p->cntr);
    } else if (dir == 1) {
      segs = segments;
      bT = bt * (p->cntr + 1);
    } else {
      segs = (segments - 1) * 2;
      if (p->cntr < segments) {
        bT = bt * (p->cntr + 1);
      } else {
        bT = bt * (segs - (p->cntr - 1));
      }
    }

    if (dynamic == 0) {
      sT = cT + bT;
    } else {
      sT = cT + (bt * segments);
    }
  } else {
    bT = bt;
    if (dir == 0) {
      segs = segments;
      cT = ct * (segments - p->cntr);
    } else if (dir == 1) {
      segs = segments;
      cT = ct * (p->cntr + 1);
    } else {
      segs = segments;
      if (p->cntr < segments) {
        cT = ct * (p->cntr + 1);
      } else {
        cT = ct * (segs - (p->cntr - 1));
      }
    }

    if (dynamic == 0) {
      sT = cT + bT;
    } else {
      sT = (ct * segments) + bT;
    }
  }

  if (change > 1 && p->cur_color == (p->num_colors - 1)) sT += lt;

  if (p->tick >= sT) {
    p->tick = 0;
    if (change == 0) {
      p->cntr += 1;
      if (p->cntr > segs) {
        p->cur_color = (p->cur_color + 1) % p->num_colors;
        p->cntr = 0;
      }
    } else if (change == 1) {
      p->cntr += 1;
      p->cur_color = (p->cur_color + 1) % p->num_colors;
      if (p->cntr > segs) {
        p->cntr = 0;
      }
    } else {
      p->cur_color += 1;
      if (p->cur_color >= p->num_colors) {
        p->cur_color = 0;
        p->cntr = 1;
        if (p->cntr > segs) {
          p->cntr = 0;
        }
      }
    }
  }

  if (doit) {
  if (p->tick < cT) {
    unpackColor(p->colors[p->cur_color], _r, _g, _b);
  } else {
    _r = _g = _b = 0;
  }
  }
}

void _fade(Pattern* p, uint8_t cst, uint8_t cbt, uint8_t est, uint8_t ebt,
           uint8_t type, uint8_t steps, bool doit) {
  int8_t c;
  uint16_t t, sT;

  sT = (cst + cbt) * steps;
  if (type == 0) {
    if (p->tick >= sT + est + ebt) {
      p->tick = 0;
      p->cur_color = (p->cur_color + 1) % p->num_colors;
    }

    if (doit) {
    if (p->tick < sT) {
      if (p->tick % (cbt + cst) < cst) {
        unpackColor(p->colors[p->cur_color], r0, g0, b0);
        morphColor(p->tick, sT, 0, 0, 0, r0, g0, b0, _r, _g, _b);
      } else {
        _r = _g = _b = 0;
      }
    } else if (p->tick < sT + est) {
      unpackColor(p->colors[p->cur_color], _r, _g, _b);
    } else {
      _r = _g = _b = 0;
    }
    }
  } else if (type == 1) {
    if (p->tick >= sT + est + ebt) {
      p->tick = 0;
      p->cur_color = (p->cur_color + 1) % p->num_colors;
    }

    if (doit) {
    if (p->tick < est) {
      unpackColor(p->colors[p->cur_color], _r, _g, _b);
    } else if (p->tick < sT + est) {
      t = p->tick - est;
      if (t % (cst + cbt) < cbt) {
        _r = _g = _b = 0;
      } else {
        unpackColor(p->colors[p->cur_color], r0, g0, b0);
        morphColor(t, sT, r0, g0, b0, 0, 0, 0, _r, _g, _b);
      }
    } else {
      _r = _g = _b = 0;
    }
    }
  } else if (type == 2) {
    if (p->tick >= sT + sT + est + ebt) {
      p->tick = 0;
      p->cur_color = (p->cur_color + 1) % p->num_colors;
    }

    if (doit) {
    if (p->tick < sT) {
      if (p->tick % (cst + cbt) < cst) {
        unpackColor(p->colors[p->cur_color], r0, g0, b0);
        morphColor(p->tick, sT, 0, 0, 0, r0, g0, b0, _r, _g, _b);
      } else {
        _r = _g = _b = 0;
      }
    } else if (p->tick < sT + est) {
      unpackColor(p->colors[p->cur_color], _r, _g, _b);
    } else if (p->tick < sT + sT + est) {
      t = p->tick - (sT + est);
      if (t % (cst + cbt) < cbt) {
        _r = _g = _b = 0;
      } else {
        unpackColor(p->colors[p->cur_color], r0, g0, b0);
        morphColor(t, sT, r0, g0, b0, 0, 0, 0, _r, _g, _b);
      }
    } else {
      _r = _g = _b = 0;
    }
    }
  } else if (type == 3) {
    if (p->tick >= sT + est + ebt) {
      p->tick = 0;
      p->cur_color = (p->cur_color + 1) % p->num_colors;
    }

    if (doit) {
    if (p->tick < est) {
      unpackColor(p->colors[p->cur_color], _r, _g, _b);
    } else if (p->tick < sT + est) {
      t = p->tick - est;
      if (t % (cst + cbt) < cbt) {
        _r = _g = _b = 0;
      } else {
        unpackColor(p->colors[p->cur_color],                       r0, g0, b0);
        unpackColor(p->colors[(p->cur_color + 1) % p->num_colors], r1, g1, b1);
        morphColor(t, sT, r0, g0, b0, r1, g1, b1, _r, _g, _b);
      }
    } else {
      _r = _g = _b = 0;
    }
    }
  } else {
    if (p->tick >= sT + est + ebt) {
      p->tick = 0;
      p->cur_color = (p->cur_color + 1) % p->num_colors;
    }

    if (doit) {
    if (p->tick < sT) {
      t = p->tick;
      if (t % (cst + cbt) < cst) {
        unpackColor(p->colors[(p->cur_color + 1) % p->num_colors], r0, g0, b0);
        unpackColor(p->colors[p->cur_color],                       r1, g1, b1);
        morphColor(t, sT, r0, g0, b0, r1, g1, b1, _r, _g, _b);
      } else {
        _r = _g = _b = 0;
      }
    } else if (p->tick < sT + est) {
      unpackColor(p->colors[p->cur_color], _r, _g, _b);
    } else {
      _r = _g = _b = 0;
    }
    }
  }
}


/*******************************************************************************
 ** PATTERN CLASS
 ******************************************************************************/
Pattern::Pattern(uint8_t _pattern, uint8_t _num_colors,
                 uint8_t c00, uint8_t c01, uint8_t c02, uint8_t c03,
                 uint8_t c04, uint8_t c05, uint8_t c06, uint8_t c07,
                 uint8_t c08, uint8_t c09, uint8_t c10, uint8_t c11,
                 uint8_t c12, uint8_t c13, uint8_t c14, uint8_t c15) {
  pattern = _pattern;
  num_colors = _num_colors;
  colors[0]  = c00; colors[1]  = c01; colors[2]  = c02; colors[3]  = c03;
  colors[4]  = c04; colors[5]  = c05; colors[6]  = c06; colors[7]  = c07;
  colors[8]  = c08; colors[9]  = c09; colors[10] = c10; colors[11] = c11;
  colors[12] = c12; colors[13] = c13; colors[14] = c14; colors[15] = c15;
}

void Pattern::load(uint16_t addr) {
  pattern    = EEPROM.read(addr + 0);
  num_colors = EEPROM.read(addr + 1);
  for (uint8_t i = 0; i < NUM_COLORS; i++) {
    colors[i]  = EEPROM.read(addr + 2 + i);
  }
}

void Pattern::save(uint16_t addr) {
  EEPROM.update(addr + 0, pattern);
  EEPROM.update(addr + 1, num_colors);
  for (uint8_t i = 0; i < NUM_COLORS; i++) {
    EEPROM.update(addr + 2 + i, colors[i]);
  }
}

void Pattern::reset() { tick = cur_color = cntr = 0; }

void Pattern::render(uint8_t& r, uint8_t& g, uint8_t& b, bool doit) {
  switch (pattern) {
    case P_RIBBON:
      _strobe(this, 25, 0, 0, 0, 0, 0, doit);
      break;
    case P_HYPER:
      _strobe(this, 50, 50, 0, 0, 0, 0, doit);
      break;
    case P_STROBE:
      _strobe(this, 9, 16, 0, 0, 0, 0, doit);
      break;
    case P_DOPS:
      _strobe(this, 3, 22, 0, 0, 0, 0, doit);
      break;
    case P_SPAZ:
      _strobe(this, 5, 45, 0, 0, 0, 0, doit);
      break;
    case P_SIGNAL:
      _strobe(this, 10, 190, 0, 0, 0, 0, doit);
      break;
    case P_BLASTER:
      _strobe(this, 5, 0, 85, 0, 0, 0, doit);
      break;
    case P_STUTTER:
      _strobe(this, 10, 5, 55, 0, 0, 0, doit);
      break;
    case P_STROBE2:
      _strobe(this, 10, 5, 55, 8, 2, 1, doit);
      break;
    case P_HYPER3:
      _strobe(this, 10, 5, 55, 4, 3, 1, doit);
      break;
    case P_DOPS3:
      _strobe(this, 10, 5, 55, 8, 3, 1, doit);
      break;
    case P_BLASTER3:
      _strobe(this, 10, 5, 55, 1, 3, 3, doit);
      break;
    case P_STUTTER3:
      _strobe(this, 10, 5, 55, 1, 3, 3, doit);
      break;
    case P_TRACER:
      _tracer(this, 5, 0, 20, 0, 1, 1, 1, 0, doit);
      break;
    case P_DASHDOPS:
      _tracer(this, 15, 0, 3, 22, 5, 0, 0, 1, doit);
      break;
    case P_DOPSDASH:
      _tracer(this, 3, 22, 50, 0, 1, 0, 0, 0, doit);
      break;
    case P_SANDWICH:
      _tracer(this, 9, 16, 9, 0, 0, 1, 0, 0, doit);
      break;
    case P_HYPENATED:
      _tracer(this, 25, 25, 25, 0, 0, 1, 0, 0, doit);
      break;
    case P_DASHED:
      _tracer(this, 10, 0, 30, 30, 1, 3, 1, 1, doit);
      break;
    case P_DOTTED:
      _tracer(this, 8, 2, 43, 0, 0, 3, 1, 0, doit);
      break;
    case P_FIREWORK:
      _tracer(this, 5, 5, 20, 20, 0, 3, 1, 0, doit);
      break;
    case P_BOTTLEROCKET:
      _tracer(this, 5, 5, 10, 40, 0, 2, 1, 0, doit);
      break;
    case P_STRETCH:
      _flux(this, 3, 0, 0, 2, 2, 1, 1, 8, doit);
      break;
    case P_DOPWAVE:
      _flux(this, 3, 1, 0, 2, 1, 0, 0, 24, doit);
      break;
    case P_SHAPESHIFT:
      _flux(this, 5, 0, 20, 2, 2, 0, 1, 4, doit);
      break;
    case P_COMET:
      _flux(this, 2, 10, 0, 0, 0, 1, 1, 10, doit);
      break;
    case P_METEOR:
      _flux(this, 2, 10, 0, 1, 0, 1, 1, 10, doit);
      break;
    case P_EMBERS:
      _flux(this, 4, 10, 0, 2, 0, 1, 1, 5, doit);
      break;
    case P_INFLUX:
      _flux(this, 5, 15, 0, 2, 2, 0, 1, 4, doit);
      break;
    case P_SWORD:
      _edge(this, 8, 0, 16, 40, 0, doit);
      break;
    case P_SWORD3:
      _edge(this, 8, 0, 16, 40, 3, doit);
      break;
    case P_BARBS:
      _edge(this, 5, 3, 16, 40, 0, doit);
      break;
    case P_BARBS3:
      _edge(this, 5, 3, 16, 40, 3, doit);
      break;
    case P_CYCLOPS:
      _edge(this, 20, 0, 3, 0, 0, doit);
      break;
    case P_FADEIN:
      _fade(this, 150, 50, 0, 0, 0, 1, doit);
      break;
    case P_STROBEIN:
      _fade(this, 25, 25, 25, 25, 0, 7, doit);
      break;
    case P_FADEOUT:
      _fade(this, 150, 50, 0, 0, 1, 1, doit);
      break;
    case P_STROBEOUT:
      _fade(this, 25, 25, 25, 25, 1, 7, doit);
      break;
    case P_PULSE:
      _fade(this, 170, 0, 0, 30, 2, 1, doit);
      break;
    case P_PULSAR:
      _fade(this, 25, 25, 50, 50, 2, 3, doit);
      break;
    case P_MORPH:
      _fade(this, 250, 0, 0, 0, 3, 1, doit);
      break;
    case P_DOPMORPH:
      _fade(this, 3, 22, 0, 0, 3, 8, doit);
      break;
    case P_SPAZMORPH:
      _fade(this, 5, 45, 0, 0, 3, 8, doit);
      break;
    case P_STROBEMORPH:
      _fade(this, 9, 16, 0, 0, 3, 8, doit);
      break;
    case P_HYPERMORPH:
      _fade(this, 50, 50, 0, 0, 3, 4, doit);
      break;
    case P_DASHMORPH:
      _fade(this, 9, 16, 34, 16, 3, 4, doit);
      break;
    case P_FUSE:
      _fade(this, 250, 0, 0, 0, 4, 1, doit);
      break;
    case P_DOPFUSE:
      _fade(this, 3, 22, 0, 0, 4, 8, doit);
      break;
    case P_SPAZFUSE:
      _fade(this, 5, 45, 0, 0, 4, 8, doit);
      break;
    case P_STROBEFUSE:
      _fade(this, 9, 16, 0, 0, 4, 8, doit);
      break;
    case P_HYPERFUSE:
      _fade(this, 50, 50, 0, 0, 4, 4, doit);
      break;
    case P_DASHFUSE:
      _fade(this, 9, 16, 34, 16, 4, 4, doit);
      break;

    default:
      break;
  }
  tick++;
  r = _r; g = _g; b = _b;
}
