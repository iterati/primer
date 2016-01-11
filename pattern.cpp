#include "pattern.h"

uint8_t _r, _g, _b, r0, g0, b0, r1, g1, b1;


/*******************************************************************************
 ** BASE ANIMATIONS
 ******************************************************************************/
void _strobe(uint8_t numc, uint8_t colors[],
    uint16_t& tick, uint8_t& cidx, int16_t& cntr,
    uint8_t st, uint8_t bt, uint8_t lt,
    uint8_t repeat, uint8_t pick, uint8_t skip, bool doit) {

  numc = constrain(numc, 1, NUM_COLORS);
  pick = (pick == 0) ? numc : pick;
  skip = (skip == 0) ? pick : skip;
  repeat = max(1, repeat);

  uint16_t sT = (st + bt) * pick;
  if (tick >= sT + lt) {
    tick = 0;
    cntr++;
    if (cntr >= repeat) {
      cntr = 0;
      cidx = cidx + skip;
      cidx = (pick == skip) ? 0 : cidx % numc;
    }
  }

  if (doit) {
    if (tick < sT) {
      if (tick % (st + bt) < bt) {
        _r = _g = _b = 0;
      } else {
        uint8_t c = (tick / (st + bt)) + cidx;
        if (c >= numc && pick == skip) {
          _r = _g = _b = 0;
        } else {
          unpackColor(colors[c % numc], _r, _g, _b);
        }
      }
    } else {
      _r = _g = _b = 0;
    }
  }
}

void _tracer(uint8_t numc, uint8_t colors[],
    uint16_t& tick, uint8_t& cidx, int16_t& cntr,
    uint8_t cst, uint8_t cbt, uint8_t tst, uint8_t tbt,
    uint8_t repeat, uint8_t pick, uint8_t skip, uint8_t pad, bool doit) {

  uint16_t tT, sT;

  pick = (pick == 0) ? (numc - 1) : pick;
  skip = (skip == 0 || skip > pick) ? pick : skip;

  if (pad == 0) {
    sT = ((cst + cbt) * pick) - cbt;
    tT = ((tbt + tst) * repeat) + tbt;
  } else {
    sT = ((cst + cbt) * pick) + cbt;
    tT = (repeat == 0) ? tbt + tst : ((tbt + tst) * repeat) - tbt;
  }

  if (tick >= sT + tT) {
    tick = 0;
    cidx += skip;
    if (cidx >= (numc - 1)) {
      cidx = (pick == skip) ? 0 : cidx % (numc - 1);
    }
  }

  if (doit) {
    if (tick < tT) {
      if (pad == 0) {
        if (tick % (tst + tbt) < tbt) _r = _g = _b = 0;
        else                          unpackColor(colors[0], _r, _g, _b);
      } else {
        if (tick % (tst + tbt) < tst) unpackColor(colors[0], _r, _g, _b);
        else                          _r = _g = _b = 0;
      }
    } else {
      uint16_t t = tick - tT;
      uint8_t c = t / (cst + cbt);
      if (pad == 0) {
        if (t % (cst + cbt) < cst) {
          if (c >= numc && pick == skip) _r = _g = _b = 0;
          else                           unpackColor(colors[(c % (numc - 1)) + 1], _r, _g, _b);
        } else {
          _r = _g = _b = 0;
        }
      } else {
        if (t % (cst + cbt) < cbt) {
          _r = _g = _b = 0;
        } else {
          if (c >= numc && pick == skip) _r = _g = _b = 0;
          else                           unpackColor(colors[(c % (numc - 1)) + 1], _r, _g, _b);
        }
      }
    }
  }
}

void _edge(uint8_t numc, uint8_t colors[],
    uint16_t& tick, uint8_t& cidx, int16_t& cntr,
    uint8_t cst, uint8_t cbt, uint8_t est, uint8_t ebt,
    uint8_t pick, bool doit) {

  pick = (pick == 0) ? numc : pick;
  uint16_t hT = (cst + cbt) * (pick - 1);

  if (tick >= (hT + hT + est + ebt)) {
    tick = 0;
    cidx += pick;
    if (cidx >= numc) cidx = 0;
  }

  if (doit) {
    uint8_t c;
    uint16_t t;
    if (tick < hT) {
      t = tick;
      if (t % (cst + cbt) < cst) {
        c = ((pick - 1) - (t / (cst + cbt))) + cidx;
      } else {
        c = -1;
      }
    } else if (tick < hT + est) {
      c = cidx;
    } else if (tick < hT + hT + est) {
      t = tick - (hT + est);
      if (t % (cst + cbt) < cst) {
        c = ((t / (cst + cbt)) + 1) + cidx;
      } else {
        c = -1;
      }
    } else {
      c = -1;
    }

    if (c == -1 || c >= numc) {
      _r = _g = _b = 0;
    } else {
      unpackColor(colors[c], _r, _g, _b);
    }
  }
}

void _flux(uint8_t numc, uint8_t colors[],
    uint16_t& tick, uint8_t& cidx, int16_t& cntr,
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

  if (change > 1 && cidx == (numc - 1)) sT += lt;

  if (tick >= sT) {
    tick = 0;
    if (change == 0) {
      cntr += 1;
      if (cntr > segs) {
        cidx = (cidx + 1) % numc;
        cntr = 0;
      }
    } else if (change == 1) {
      cntr += 1;
      cidx = (cidx + 1) % numc;
      if (cntr > segs) {
        cntr = 0;
      }
    } else {
      cidx += 1;
      if (cidx >= numc) {
        cidx = 0;
        cntr += 1;
        if (cntr > segs) {
          cntr = 0;
        }
      }
    }
  }

  if (doit) {
    if (tick < cT) unpackColor(colors[cidx], _r, _g, _b);
    else           _r = _g = _b = 0;
  }
}

void _fade(uint8_t numc, uint8_t colors[],
    uint16_t& tick, uint8_t& cidx, int16_t& cntr,
    uint8_t cst, uint8_t cbt, uint8_t est, uint8_t ebt,
    uint8_t type, uint8_t steps, bool doit) {

  int8_t c;
  uint16_t t, sT;

  sT = (cst + cbt) * steps;
  if (type == 0) {
    if (tick >= sT + est + ebt) {
      tick = 0;
      cidx = (cidx + 1) % numc;
    }

    if (doit) {
      if (tick < sT) {
        if (tick % (cbt + cst) < cst) {
          unpackColor(colors[cidx], r0, g0, b0);
          morphColor(tick, sT, 0, 0, 0, r0, g0, b0, _r, _g, _b);
        } else {
          _r = _g = _b = 0;
        }
      } else if (tick < sT + est) {
        unpackColor(colors[cidx], _r, _g, _b);
      } else {
        _r = _g = _b = 0;
      }
    }
  } else if (type == 1) {
    if (tick >= sT + est + ebt) {
      tick = 0;
      cidx = (cidx + 1) % numc;
    }

    if (doit) {
      if (tick < est) {
        unpackColor(colors[cidx], _r, _g, _b);
      } else if (tick < sT + est) {
        t = tick - est;
        if (t % (cst + cbt) < cbt) {
          _r = _g = _b = 0;
        } else {
          unpackColor(colors[cidx], r0, g0, b0);
          morphColor(t, sT, r0, g0, b0, 0, 0, 0, _r, _g, _b);
        }
      } else {
        _r = _g = _b = 0;
      }
    }
  } else if (type == 2) {
    if (tick >= sT + sT + est + ebt) {
      tick = 0;
      cidx = (cidx + 1) % numc;
    }

    if (doit) {
      if (tick < sT) {
        if (tick % (cst + cbt) < cst) {
          unpackColor(colors[cidx], r0, g0, b0);
          morphColor(tick, sT, 0, 0, 0, r0, g0, b0, _r, _g, _b);
        } else {
          _r = _g = _b = 0;
        }
      } else if (tick < sT + est) {
        unpackColor(colors[cidx], _r, _g, _b);
      } else if (tick < sT + sT + est) {
        t = tick - (sT + est);
        if (t % (cst + cbt) < cbt) {
          _r = _g = _b = 0;
        } else {
          unpackColor(colors[cidx], r0, g0, b0);
          morphColor(t, sT, r0, g0, b0, 0, 0, 0, _r, _g, _b);
        }
      } else {
        _r = _g = _b = 0;
      }
    }
  } else if (type == 3) {
    if (tick >= sT + est + ebt) {
      tick = 0;
      cidx = (cidx + 1) % numc;
    }

    if (doit) {
      if (tick < est) {
        unpackColor(colors[cidx], _r, _g, _b);
      } else if (tick < sT + est) {
        t = tick - est;
        if (t % (cst + cbt) < cbt) {
          _r = _g = _b = 0;
        } else {
          unpackColor(colors[cidx],                       r0, g0, b0);
          unpackColor(colors[(cidx + 1) % numc], r1, g1, b1);
          morphColor(t, sT, r0, g0, b0, r1, g1, b1, _r, _g, _b);
        }
      } else {
        _r = _g = _b = 0;
      }
    }
  } else {
    if (tick >= sT + est + ebt) {
      tick = 0;
      cidx = (cidx + 1) % numc;
    }

    if (doit) {
      if (tick < sT) {
        t = tick;
        if (t % (cst + cbt) < cst) {
          unpackColor(colors[(cidx + 1) % numc], r0, g0, b0);
          unpackColor(colors[cidx],                       r1, g1, b1);
          morphColor(t, sT, r0, g0, b0, r1, g1, b1, _r, _g, _b);
        } else {
          _r = _g = _b = 0;
        }
      } else if (tick < sT + est) {
        unpackColor(colors[cidx], _r, _g, _b);
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
    uint8_t pattern, uint8_t numc, uint8_t colors[],
    uint16_t& tick, uint8_t& cidx, int16_t& cntr,
    uint8_t& r, uint8_t& g, uint8_t& b, bool doit) {
  switch (pattern) {
    case P_RIBBON:
      _strobe(numc, colors, tick, cidx, cntr, 25, 0, 0, 0, 0, 0, doit);
      break;
    case P_HYPER:
      _strobe(numc, colors, tick, cidx, cntr, 50, 50, 0, 0, 0, 0, doit);
      break;
    case P_STROBE:
      _strobe(numc, colors, tick, cidx, cntr, 9, 16, 0, 0, 0, 0, doit);
      break;
    case P_NANO:
      _strobe(numc, colors, tick, cidx, cntr, 1, 24, 0, 0, 0, 0, doit);
      break;
    case P_DOPS:
      _strobe(numc, colors, tick, cidx, cntr, 3, 22, 0, 0, 0, 0, doit);
      break;
    case P_SLOW_STROBE:
      _strobe(numc, colors, tick, cidx, cntr, 20, 30, 0, 0, 0, 0, doit);
      break;
    case P_STROBIE:
      _strobe(numc, colors, tick, cidx, cntr, 5, 45, 0, 0, 0, 0, doit);
      break;
    case P_FAINT:
      _strobe(numc, colors, tick, cidx, cntr, 3, 97, 0, 0, 0, 0, doit);
      break;
    case P_SIGNAL:
      _strobe(numc, colors, tick, cidx, cntr, 10, 190, 0, 0, 0, 0, doit);
      break;
    case P_BLASTER:
      _strobe(numc, colors, tick, cidx, cntr, 5, 0, 140, 0, 0, 0, doit);
      break;
    case P_HEAVYBLASTER:
      _strobe(numc, colors, tick, cidx, cntr, 10, 0, 140, 0, 0, 0, doit);
      break;
    case P_AUTOBLASTER:
      _strobe(numc, colors, tick, cidx, cntr, 5, 10, 140, 0, 0, 0, doit);
      break;
    case P_STROBE2:
      _strobe(numc, colors, tick, cidx, cntr, 9, 16, 0, 32, 2, 1, doit);
      break;
    case P_HYPER3:
      _strobe(numc, colors, tick, cidx, cntr, 50, 50, 0, 16, 3, 1, doit);
      break;
    case P_DOPS3:
      _strobe(numc, colors, tick, cidx, cntr, 3, 22, 0, 64, 3, 1, doit);
      break;
    case P_BLASTER3:
      _strobe(numc, colors, tick, cidx, cntr, 5, 0, 140, 0, 3, 3, doit);
      break;
    case P_HEAVYBLASTER3:
      _strobe(numc, colors, tick, cidx, cntr, 10, 0, 140, 0, 3, 3, doit);
      break;
    case P_AUTOBLASTER3:
      _strobe(numc, colors, tick, cidx, cntr, 5, 10, 140, 0, 3, 3, doit);
      break;
    case P_TRACER:
      _tracer(numc, colors, tick, cidx, cntr, 5, 0, 20, 0, 1, 1, 1, 0, doit);
      break;
    case P_DASHDOPS:
      _tracer(numc, colors, tick, cidx, cntr, 5, 0, 3, 22, 5, 0, 0, 1, doit);
      break;
    case P_DOPSDASH:
      _tracer(numc, colors, tick, cidx, cntr, 3, 22, 50, 0, 1, 0, 0, 1, doit);
      break;
    case P_VEXING:
      _tracer(numc, colors, tick, cidx, cntr, 5, 45, 3, 22, 5, 1, 1, 0, doit);
      break;
    case P_VEXING3:
      _tracer(numc, colors, tick, cidx, cntr, 5, 45, 3, 22, 5, 3, 1, 0, doit);
      break;
    case P_RIBBONTRACER:
      _tracer(numc, colors, tick, cidx, cntr, 10, 0, 30, 30, 1, 3, 1, 1, doit);
      break;
    case P_DOTTED:
      _tracer(numc, colors, tick, cidx, cntr, 8, 12, 45, 0, 0, 3, 1, 0, doit);
      break;
    case P_FIREWORK:
      _tracer(numc, colors, tick, cidx, cntr, 5, 30, 40, 60, 0, 3, 1, 0, doit);
      break;
    case P_BOTTLEROCKET:
      _tracer(numc, colors, tick, cidx, cntr, 5, 45, 15, 45, 0, 2, 1, 0, doit);
      break;
    case P_GROW:
      _flux(numc, colors, tick, cidx, cntr, 3, 0, 0, 1, 2, 1, 1, 16, doit);
      break;
    case P_SHRINK:
      _flux(numc, colors, tick, cidx, cntr, 3, 0, 0, 0, 2, 1, 1, 16, doit);
      break;
    case P_STRETCH:
      _flux(numc, colors, tick, cidx, cntr, 3, 0, 0, 2, 2, 1, 1, 16, doit);
      break;
    case P_WAVE:
      _flux(numc, colors, tick, cidx, cntr, 5, 2, 0, 2, 1, 0, 0, 32, doit);
      break;
    case P_SHIFT:
      _flux(numc, colors, tick, cidx, cntr, 5, 0, 80, 2, 2, 0, 1, 8, doit);
      break;
    case P_COMET:
      _flux(numc, colors, tick, cidx, cntr, 3, 30, 0, 0, 0, 1, 1, 32, doit);
      break;
    case P_METEOR:
      _flux(numc, colors, tick, cidx, cntr, 3, 30, 0, 1, 0, 1, 1, 32, doit);
      break;
    case P_EMBERS:
      _flux(numc, colors, tick, cidx, cntr, 3, 30, 0, 2, 0, 1, 1, 32, doit);
      break;
    case P_INFLUX:
      _flux(numc, colors, tick, cidx, cntr, 5, 20, 0, 2, 2, 0, 1, 8, doit);
      break;
    case P_SWORD:
      _edge(numc, colors, tick, cidx, cntr, 8, 0, 16, 40, 0, doit);
      break;
    case P_SWORD5:
      _edge(numc, colors, tick, cidx, cntr, 8, 0, 16, 40, 5, doit);
      break;
    case P_RAZOR:
      _edge(numc, colors, tick, cidx, cntr, 3, 0, 8, 85, 0, doit);
      break;
    case P_RAZOR5:
      _edge(numc, colors, tick, cidx, cntr, 3, 0, 8, 85, 5, doit);
      break;
    case P_BARBS:
      _edge(numc, colors, tick, cidx, cntr, 5, 5, 16, 40, 0, doit);
      break;
    case P_BARBS5:
      _edge(numc, colors, tick, cidx, cntr, 5, 5, 16, 40, 5, doit);
      break;
    case P_CYCLOPS:
      _edge(numc, colors, tick, cidx, cntr, 20, 0, 3, 0, 0, doit);
      break;
    case P_FADEIN:
      _fade(numc, colors, tick, cidx, cntr, 150, 100, 0, 0, 0, 1, doit);
      break;
    case P_STROBEIN:
      _fade(numc, colors, tick, cidx, cntr, 25, 25, 25, 75, 0, 7, doit);
      break;
    case P_FADEOUT:
      _fade(numc, colors, tick, cidx, cntr, 150, 100, 0, 0, 1, 1, doit);
      break;
    case P_STROBEOUT:
      _fade(numc, colors, tick, cidx, cntr, 25, 25, 25, 75, 1, 7, doit);
      break;
    case P_PULSE:
      _fade(numc, colors, tick, cidx, cntr, 150, 0, 0, 150, 2, 1, doit);
      break;
    case P_PULSAR:
      _fade(numc, colors, tick, cidx, cntr, 25, 25, 50, 150, 2, 4, doit);
      break;
    case P_SLOWMORPH:
      _fade(numc, colors, tick, cidx, cntr, 250, 0, 0, 0, 3, 8, doit);
      break;
    case P_MORPH:
      _fade(numc, colors, tick, cidx, cntr, 250, 0, 0, 0, 3, 1, doit);
      break;
    case P_DOPMORPH:
      _fade(numc, colors, tick, cidx, cntr, 3, 22, 3, 22, 3, 3, doit);
      break;
    case P_STROBIEMORPH:
      _fade(numc, colors, tick, cidx, cntr, 5, 45, 5, 45, 3, 3, doit);
      break;
    case P_STROBEMORPH:
      _fade(numc, colors, tick, cidx, cntr, 9, 16, 9, 16, 3, 3, doit);
      break;
    case P_HYPERMORPH:
      _fade(numc, colors, tick, cidx, cntr, 50, 50, 50, 50, 3, 3, doit);
      break;
    case P_DASHMORPH:
      _fade(numc, colors, tick, cidx, cntr, 5, 45, 50, 45, 3, 3, doit);
      break;
    case P_FUSE:
      _fade(numc, colors, tick, cidx, cntr, 250, 0, 0, 0, 4, 1, doit);
      break;
    case P_DOPFUSE:
      _fade(numc, colors, tick, cidx, cntr, 3, 22, 3, 22, 4, 3, doit);
      break;
    case P_STROBIEFUSE:
      _fade(numc, colors, tick, cidx, cntr, 5, 45, 5, 45, 4, 3, doit);
      break;
    case P_STROBEFUSE:
      _fade(numc, colors, tick, cidx, cntr, 9, 16, 9, 16, 4, 3, doit);
      break;
    case P_HYPERFUSE:
      _fade(numc, colors, tick, cidx, cntr, 50, 50, 50, 50, 4, 3, doit);
      break;
    case P_DASHFUSE:
      _fade(numc, colors, tick, cidx, cntr, 5, 45, 50, 45, 4, 3, doit);
      break;

    default:
      break;
  }
  tick++;
  r = _r; g = _g; b = _b;
}
