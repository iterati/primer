#include "pattern.h"

uint8_t _r, _g, _b, r0, g0, b0, r1, g1, b1;


/*******************************************************************************
 ** BASE ANIMATIONS
 ******************************************************************************/
void _strobe(uint8_t num_colors, uint8_t colors[],
    uint16_t& tick, uint8_t& cur_color, int16_t& cntr,
    uint8_t ct, uint8_t bt, uint8_t lt,
    uint8_t repeat, uint8_t pick, uint8_t skip, bool doit) {
  int8_t c;
  uint16_t sT;

  repeat = (repeat == 0) ? 1 : repeat;
  pick = (pick == 0) ? num_colors : pick;
  skip = (skip == 0 || skip > pick) ? pick : skip;

  sT = ((ct + bt) * pick) * repeat;
  if (pick == 2 && skip == 1) sT += ct + bt;

  if (tick >= sT + lt) {
    tick = 0;
    cur_color += skip;
    if (cur_color >= num_colors) {
      if (pick == skip) {
        cur_color = 0;
      } else {
        cur_color %= num_colors;
      }
    }
  }

  if (doit) {
  if (tick < sT) {
    if (tick % (ct + bt) < ct) {
      c = ((tick / (ct + bt)) % pick) + cur_color;
      if (pick == skip) {
        if (c >= num_colors) {
          _r = _g = _b = 0;
        } else {
          unpackColor(colors[c], _r, _g, _b);
        }
      } else {
        unpackColor(colors[c % num_colors], _r, _g, _b);
      }
    } else {
      _r = _g = _b = 0;
    }
  } else {
    _r = _g = _b = 0;
  }
  }
}

void _tracer(uint8_t num_colors, uint8_t colors[],
    uint16_t& tick, uint8_t& cur_color, int16_t& cntr,
    uint8_t cst, uint8_t cbt, uint8_t tst, uint8_t tbt,
    uint8_t repeat, uint8_t pick, uint8_t skip, uint8_t pad, bool doit) {
  int8_t c;
  uint16_t t, tT, oT;

  pick = (pick == 0) ? (num_colors - 1) : pick;
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

  if (tick >= oT + tT) {
    tick = 0;
    cur_color += skip;
    if (cur_color >= (num_colors - 1)) {
      if (pick == skip) {
        cur_color = 0;
      } else {
        cur_color = cur_color % (num_colors - 1);
      }
    }
  }

  if (doit) {
  if (tick < oT) {
    if (pad == 0) {
      if (tick % (cst + cbt) < cbt) {
        c = -1;
      } else {
        c = (tick / (cbt + cst)) + cur_color + 1;
      }
    } else {
      if (tick % (cst + cbt) < cst) {
        c = (tick / (cbt + cst)) + cur_color + 1;
      } else {
        c = -1;
      }
    }
  } else {
    t = tick - oT;
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
    unpackColor(colors[0], _r, _g, _b);
  } else {
    if (pick == skip) {
      if (c > num_colors) {
        _r = _g = _b = 0;
      } else {
        unpackColor(colors[c], _r, _g, _b);
      }
    } else {
      unpackColor(colors[(c % (num_colors - 1)) + 1], _r, _g, _b);
    }
  }
  }
}

void _edge(uint8_t num_colors, uint8_t colors[],
    uint16_t& tick, uint8_t& cur_color, int16_t& cntr,
    uint8_t cst, uint8_t cbt, uint8_t est, uint8_t ebt,
    uint8_t pick, bool doit) {
  int8_t c;
  uint16_t t, hT;

  pick = (pick == 0) ? num_colors : pick;
  hT = (cst + cbt) * (pick - 1);

  if (tick >= (hT + hT + est + ebt)) {
    tick = 0;
    cur_color += pick;
    if (cur_color >= num_colors) {
      cur_color = 0;
    }
  }

  if (doit) {
  if (tick < hT) {
    t = tick;
    if (t % (cst + cbt) < cst) {
      c = ((pick - 1) - (t / (cst + cbt))) + cur_color;
    } else {
      c = -1;
    }
  } else if (tick < hT + est) {
    c = cur_color;
  } else if (tick < hT + hT + est) {
    t = tick - (hT + est);
    if (t % (cst + cbt) < cst) {
      c = ((t / (cst + cbt)) + 1) + cur_color;
    } else {
      c = -1;
    }
  } else {
    c = -1;
  }

  if (c == -1 || c >= num_colors) {
    _r = _g = _b = 0;
  } else {
    unpackColor(colors[c], _r, _g, _b);
  }
  }
}

void _flux(uint8_t num_colors, uint8_t colors[],
    uint16_t& tick, uint8_t& cur_color, int16_t& cntr,
    uint8_t ct, uint8_t bt, uint8_t lt, uint8_t dir,
    uint8_t change, uint8_t dynamic, uint8_t target, uint8_t segments, bool doit) {
  int8_t c;
  uint16_t t, cT, bT, sT;
  uint8_t segs;

  if (target == 0) {
    cT = ct;
    if (dir == 0) {
      segs = segments;
      bT = bt * (segments - cntr);
    } else if (dir == 1) {
      segs = segments;
      bT = bt * (cntr + 1);
    } else {
      segs = (segments - 1) * 2;
      if (cntr < segments) {
        bT = bt * (cntr + 1);
      } else {
        bT = bt * (segs - (cntr - 1));
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
      cT = ct * (segments - cntr);
    } else if (dir == 1) {
      segs = segments;
      cT = ct * (cntr + 1);
    } else {
      segs = (segments - 1) * 2;
      if (cntr < segments) {
        cT = ct * (cntr + 1);
      } else {
        cT = ct * (segs - (cntr - 1));
      }
    }

    if (dynamic == 0) {
      sT = cT + bT;
    } else {
      sT = (ct * segments) + bT;
    }
  }

  if (change > 1 && cur_color == (num_colors - 1)) sT += lt;

  if (tick >= sT) {
    tick = 0;
    if (change == 0) {
      cntr += 1;
      if (cntr > segs) {
        cur_color = (cur_color + 1) % num_colors;
        cntr = 0;
      }
    } else if (change == 1) {
      cntr += 1;
      cur_color = (cur_color + 1) % num_colors;
      if (cntr > segs) {
        cntr = 0;
      }
    } else {
      cur_color += 1;
      if (cur_color >= num_colors) {
        cur_color = 0;
        cntr += 1;
        if (cntr > segs) {
          cntr = 0;
        }
      }
    }
  }

  if (doit) {
  if (tick < cT) {
    unpackColor(colors[cur_color], _r, _g, _b);
  } else {
    _r = _g = _b = 0;
  }
  }
}

void _fade(uint8_t num_colors, uint8_t colors[],
    uint16_t& tick, uint8_t& cur_color, int16_t& cntr,
    uint8_t cst, uint8_t cbt, uint8_t est, uint8_t ebt,
    uint8_t type, uint8_t steps, bool doit) {
  int8_t c;
  uint16_t t, sT;

  sT = (cst + cbt) * steps;
  if (type == 0) {
    if (tick >= sT + est + ebt) {
      tick = 0;
      cur_color = (cur_color + 1) % num_colors;
    }

    if (doit) {
    if (tick < sT) {
      if (tick % (cbt + cst) < cst) {
        unpackColor(colors[cur_color], r0, g0, b0);
        morphColor(tick, sT, 0, 0, 0, r0, g0, b0, _r, _g, _b);
      } else {
        _r = _g = _b = 0;
      }
    } else if (tick < sT + est) {
      unpackColor(colors[cur_color], _r, _g, _b);
    } else {
      _r = _g = _b = 0;
    }
    }
  } else if (type == 1) {
    if (tick >= sT + est + ebt) {
      tick = 0;
      cur_color = (cur_color + 1) % num_colors;
    }

    if (doit) {
    if (tick < est) {
      unpackColor(colors[cur_color], _r, _g, _b);
    } else if (tick < sT + est) {
      t = tick - est;
      if (t % (cst + cbt) < cbt) {
        _r = _g = _b = 0;
      } else {
        unpackColor(colors[cur_color], r0, g0, b0);
        morphColor(t, sT, r0, g0, b0, 0, 0, 0, _r, _g, _b);
      }
    } else {
      _r = _g = _b = 0;
    }
    }
  } else if (type == 2) {
    if (tick >= sT + sT + est + ebt) {
      tick = 0;
      cur_color = (cur_color + 1) % num_colors;
    }

    if (doit) {
    if (tick < sT) {
      if (tick % (cst + cbt) < cst) {
        unpackColor(colors[cur_color], r0, g0, b0);
        morphColor(tick, sT, 0, 0, 0, r0, g0, b0, _r, _g, _b);
      } else {
        _r = _g = _b = 0;
      }
    } else if (tick < sT + est) {
      unpackColor(colors[cur_color], _r, _g, _b);
    } else if (tick < sT + sT + est) {
      t = tick - (sT + est);
      if (t % (cst + cbt) < cbt) {
        _r = _g = _b = 0;
      } else {
        unpackColor(colors[cur_color], r0, g0, b0);
        morphColor(t, sT, r0, g0, b0, 0, 0, 0, _r, _g, _b);
      }
    } else {
      _r = _g = _b = 0;
    }
    }
  } else if (type == 3) {
    if (tick >= sT + est + ebt) {
      tick = 0;
      cur_color = (cur_color + 1) % num_colors;
    }

    if (doit) {
    if (tick < est) {
      unpackColor(colors[cur_color], _r, _g, _b);
    } else if (tick < sT + est) {
      t = tick - est;
      if (t % (cst + cbt) < cbt) {
        _r = _g = _b = 0;
      } else {
        unpackColor(colors[cur_color],                       r0, g0, b0);
        unpackColor(colors[(cur_color + 1) % num_colors], r1, g1, b1);
        morphColor(t, sT, r0, g0, b0, r1, g1, b1, _r, _g, _b);
      }
    } else {
      _r = _g = _b = 0;
    }
    }
  } else {
    if (tick >= sT + est + ebt) {
      tick = 0;
      cur_color = (cur_color + 1) % num_colors;
    }

    if (doit) {
    if (tick < sT) {
      t = tick;
      if (t % (cst + cbt) < cst) {
        unpackColor(colors[(cur_color + 1) % num_colors], r0, g0, b0);
        unpackColor(colors[cur_color],                       r1, g1, b1);
        morphColor(t, sT, r0, g0, b0, r1, g1, b1, _r, _g, _b);
      } else {
        _r = _g = _b = 0;
      }
    } else if (tick < sT + est) {
      unpackColor(colors[cur_color], _r, _g, _b);
    } else {
      _r = _g = _b = 0;
    }
    }
  }
}


/*******************************************************************************
 ** PATTERNS
 ******************************************************************************/
void renderPattern(
    uint8_t pattern, uint8_t num_colors, uint8_t colors[],
    uint16_t& tick, uint8_t& cur_color, int16_t& cntr,
    uint8_t& r, uint8_t& g, uint8_t& b, bool doit) {
  switch (pattern) {
    case P_RIBBON:
      _strobe(num_colors, colors, tick, cur_color, cntr, 25, 0, 0, 0, 0, 0, doit);
      break;
    case P_HYPER:
      _strobe(num_colors, colors, tick, cur_color, cntr, 50, 50, 0, 0, 0, 0, doit);
      break;
    case P_STROBE:
      _strobe(num_colors, colors, tick, cur_color, cntr, 9, 16, 0, 0, 0, 0, doit);
      break;
    case P_NANO:
      _strobe(num_colors, colors, tick, cur_color, cntr, 1, 24, 0, 0, 0, 0, doit);
      break;
    case P_DOPS:
      _strobe(num_colors, colors, tick, cur_color, cntr, 3, 22, 0, 0, 0, 0, doit);
      break;
    case P_STROBIE:
      _strobe(num_colors, colors, tick, cur_color, cntr, 5, 45, 0, 0, 0, 0, doit);
      break;
    case P_FAINT:
      _strobe(num_colors, colors, tick, cur_color, cntr, 3, 97, 0, 0, 0, 0, doit);
      break;
    case P_SIGNAL:
      _strobe(num_colors, colors, tick, cur_color, cntr, 10, 190, 0, 0, 0, 0, doit);
      break;
    case P_BLASTER:
      _strobe(num_colors, colors, tick, cur_color, cntr, 5, 0, 140, 0, 0, 0, doit);
      break;
    case P_HEAVYBLASTER:
      _strobe(num_colors, colors, tick, cur_color, cntr, 10, 0, 140, 0, 0, 0, doit);
      break;
    case P_AUTOBLASTER:
      _strobe(num_colors, colors, tick, cur_color, cntr, 5, 10, 140, 0, 0, 0, doit);
      break;
    case P_STROBE2:
      _strobe(num_colors, colors, tick, cur_color, cntr, 9, 16, 0, 8, 2, 1, doit);
      break;
    case P_HYPER3:
      _strobe(num_colors, colors, tick, cur_color, cntr, 25, 25, 0, 8, 3, 1, doit);
      break;
    case P_DOPS3:
      _strobe(num_colors, colors, tick, cur_color, cntr, 3, 22, 0, 8, 3, 1, doit);
      break;
    case P_BLASTER3:
      _strobe(num_colors, colors, tick, cur_color, cntr, 5, 0, 140, 0, 3, 3, doit);
      break;
    case P_HEAVYBLASTER3:
      _strobe(num_colors, colors, tick, cur_color, cntr, 10, 0, 140, 0, 3, 3, doit);
      break;
    case P_AUTOBLASTER3:
      _strobe(num_colors, colors, tick, cur_color, cntr, 5, 10, 140, 0, 3, 3, doit);
      break;
    case P_TRACER:
      _tracer(num_colors, colors, tick, cur_color, cntr, 5, 0, 20, 0, 1, 1, 1, 0, doit);
      break;
    case P_DASHDOPS:
      _tracer(num_colors, colors, tick, cur_color, cntr, 5, 0, 3, 22, 5, 0, 0, 1, doit);
      break;
    case P_DOPSDASH:
      _tracer(num_colors, colors, tick, cur_color, cntr, 3, 22, 50, 0, 1, 0, 0, 0, doit);
      break;
    case P_STROBETRACER:
      _tracer(num_colors, colors, tick, cur_color, cntr, 9, 16, 9, 0, 0, 1, 0, 0, doit);
      break;
    case P_HYPERTRACER:
      _tracer(num_colors, colors, tick, cur_color, cntr, 25, 25, 25, 0, 0, 1, 0, 0, doit);
      break;
    case P_RIBBONTRACER:
      _tracer(num_colors, colors, tick, cur_color, cntr, 10, 0, 30, 30, 1, 3, 1, 1, doit);
      break;
    case P_DOTTED:
      _tracer(num_colors, colors, tick, cur_color, cntr, 8, 12, 43, 0, 0, 3, 1, 0, doit);
      break;
    case P_FIREWORK:
      _tracer(num_colors, colors, tick, cur_color, cntr, 5, 30, 40, 60, 0, 3, 1, 0, doit);
      break;
    case P_BOTTLEROCKET:
      _tracer(num_colors, colors, tick, cur_color, cntr, 5, 45, 15, 45, 0, 2, 1, 0, doit);
      break;
    case P_GROW:
      _flux(num_colors, colors, tick, cur_color, cntr, 3, 0, 0, 0, 2, 1, 1, 16, doit);
      break;
    case P_SHRINK:
      _flux(num_colors, colors, tick, cur_color, cntr, 3, 0, 0, 1, 2, 1, 1, 16, doit);
      break;
    case P_SPRING:
      _flux(num_colors, colors, tick, cur_color, cntr, 3, 0, 0, 2, 2, 1, 1, 16, doit);
      break;
    case P_WAVE:
      _flux(num_colors, colors, tick, cur_color, cntr, 5, 2, 0, 2, 1, 0, 0, 64, doit);
      break;
    case P_SHAPESHIFT:
      _flux(num_colors, colors, tick, cur_color, cntr, 5, 0, 80, 2, 2, 0, 1, 8, doit);
      break;
    case P_COMET:
      _flux(num_colors, colors, tick, cur_color, cntr, 3, 30, 60, 0, 0, 1, 1, 32, doit);
      break;
    case P_METEOR:
      _flux(num_colors, colors, tick, cur_color, cntr, 3, 30, 60, 1, 0, 1, 1, 32, doit);
      break;
    case P_EMBERS:
      _flux(num_colors, colors, tick, cur_color, cntr, 3, 30, 90, 2, 0, 1, 1, 32, doit);
      break;
    case P_INFLUX:
      _flux(num_colors, colors, tick, cur_color, cntr, 5, 20, 0, 2, 2, 0, 1, 4, doit);
      break;
    case P_SWORD:
      _edge(num_colors, colors, tick, cur_color, cntr, 8, 0, 16, 40, 0, doit);
      break;
    case P_SWORD5:
      _edge(num_colors, colors, tick, cur_color, cntr, 8, 0, 16, 40, 5, doit);
      break;
    case P_RAZOR:
      _edge(num_colors, colors, tick, cur_color, cntr, 3, 0, 8, 85, 0, doit);
      break;
    case P_RAZOR5:
      _edge(num_colors, colors, tick, cur_color, cntr, 3, 0, 8, 85, 5, doit);
      break;
    case P_BARBS:
      _edge(num_colors, colors, tick, cur_color, cntr, 5, 5, 16, 40, 0, doit);
      break;
    case P_BARBS5:
      _edge(num_colors, colors, tick, cur_color, cntr, 5, 5, 16, 40, 5, doit);
      break;
    case P_CYCLOPS:
      _edge(num_colors, colors, tick, cur_color, cntr, 20, 0, 3, 0, 0, doit);
      break;
    case P_FADEIN:
      _fade(num_colors, colors, tick, cur_color, cntr, 150, 100, 0, 0, 0, 1, doit);
      break;
    case P_STROBEIN:
      _fade(num_colors, colors, tick, cur_color, cntr, 25, 25, 25, 75, 0, 7, doit);
      break;
    case P_FADEOUT:
      _fade(num_colors, colors, tick, cur_color, cntr, 150, 100, 0, 0, 1, 1, doit);
      break;
    case P_STROBEOUT:
      _fade(num_colors, colors, tick, cur_color, cntr, 25, 25, 25, 75, 1, 7, doit);
      break;
    case P_PULSE:
      _fade(num_colors, colors, tick, cur_color, cntr, 150, 0, 0, 150, 2, 1, doit);
      break;
    case P_PULSAR:
      _fade(num_colors, colors, tick, cur_color, cntr, 25, 25, 50, 150, 2, 4, doit);
      break;
    case P_MORPH:
      _fade(num_colors, colors, tick, cur_color, cntr, 250, 0, 0, 0, 3, 1, doit);
      break;
    case P_DOPMORPH:
      _fade(num_colors, colors, tick, cur_color, cntr, 3, 22, 0, 0, 3, 8, doit);
      break;
    case P_STROBIEMORPH:
      _fade(num_colors, colors, tick, cur_color, cntr, 5, 45, 0, 0, 3, 8, doit);
      break;
    case P_STROBEMORPH:
      _fade(num_colors, colors, tick, cur_color, cntr, 9, 16, 0, 0, 3, 8, doit);
      break;
    case P_HYPERMORPH:
      _fade(num_colors, colors, tick, cur_color, cntr, 50, 50, 0, 0, 3, 4, doit);
      break;
    case P_DASHMORPH:
      _fade(num_colors, colors, tick, cur_color, cntr, 5, 45, 50, 45, 3, 4, doit);
      break;
    case P_FUSE:
      _fade(num_colors, colors, tick, cur_color, cntr, 250, 0, 0, 0, 4, 1, doit);
      break;
    case P_DOPFUSE:
      _fade(num_colors, colors, tick, cur_color, cntr, 3, 22, 0, 0, 4, 4, doit);
      break;
    case P_STROBIEFUSE:
      _fade(num_colors, colors, tick, cur_color, cntr, 5, 45, 5, 45, 4, 3, doit);
      break;
    case P_STROBEFUSE:
      _fade(num_colors, colors, tick, cur_color, cntr, 9, 16, 0, 0, 4, 4, doit);
      break;
    case P_HYPERFUSE:
      _fade(num_colors, colors, tick, cur_color, cntr, 50, 50, 0, 0, 4, 4, doit);
      break;
    case P_DASHFUSE:
      _fade(num_colors, colors, tick, cur_color, cntr, 5, 45, 50, 45, 4, 4, doit);
      break;

    default:
      break;
  }
  tick++;
  r = _r; g = _g; b = _b;
}
