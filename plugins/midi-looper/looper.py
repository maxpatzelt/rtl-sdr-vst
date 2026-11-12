#!/usr/bin/env python3
"""
MIDI Looper — mod wheel controlled, two independent buffers.

MOD WHEEL (CC#01):
  127 (full)      -> IDLE: arm  |  PLAYING: enter dub
  1-126 (mid)     -> RECORDING: stop, start looping
                     DUBBING: commit and back to playing
  0 (brief)       -> undo last committed change (notes or CC, in order)
                     DUBBING: cancel dub without committing
  0 (hold 8 beats)-> clear everything and stop all playback

DUB MODE — first message determines buffer:
  note first -> note dub: layers onto notes buffer (same loop length)
  CC first   -> CC dub: records into CC buffer with its own length

BUFFERS:
  notes buffer — length fixed at end of initial RECORDING
  CC buffer    — length fixed at end of first CC dub session
  Both loop independently and simultaneously.
"""

import mido
import threading
import time
import collections

IN_PORT  = 'UM-ONE:UM-ONE MIDI 1 24:0'
OUT_PORT = 'UM-ONE:UM-ONE MIDI 1 24:0'

PPQN = 24

# --- global state ---
state          = 'IDLE'
rec_start_time = 0.0
lock           = threading.Lock()
out            = None

# --- BPM tracking (clock only, not used for loop timing) ---
_clock_intervals = collections.deque(maxlen=48)
_last_clock_ts   = None
current_bpm      = 120.0

# --- notes buffer ---
notes_loop     = []
notes_duration = 0.0
notes_overdub  = []
notes_stop     = threading.Event()

# --- CC buffer ---
cc_loop        = []
cc_duration    = 0.0
cc_overdub     = []
cc_dub_start   = None
cc_stop        = threading.Event()

# --- dub ---
dub_type       = None   # None | 'notes' | 'cc'

# --- undo stack: snapshots of (notes_loop, cc_loop, cc_duration) BEFORE each commit ---
undo_stack     = []

# --- mod wheel zero hold detection ---
_zero_timer    = None
_was_at_zero   = False

def ts():
    return time.perf_counter()

# ── Undo ──────────────────────────────────────────────────────────────────────

def push_undo():
    """Call while holding lock, BEFORE modifying loop state."""
    undo_stack.append({
        'notes_loop':     list(notes_loop),
        'notes_duration': notes_duration,
        'cc_loop':        list(cc_loop),
        'cc_duration':    cc_duration,
    })

def do_undo():
    """Undo last committed change. Call while holding lock."""
    global state, notes_loop, notes_duration, cc_loop, cc_duration
    global notes_overdub, cc_overdub, cc_dub_start, dub_type

    if state == 'DUBBING':
        # cancel current dub without committing
        notes_overdub = []
        cc_overdub    = []
        cc_dub_start  = None
        dub_type      = None
        state         = 'PLAYING'
        print('  dub cancelled → playing', flush=True)
        return

    if not undo_stack:
        print('  nothing to undo', flush=True)
        return

    snap = undo_stack.pop()
    prev_notes_dur = snap['notes_duration']
    prev_cc_dur    = snap['cc_duration']
    curr_cc_dur    = cc_duration

    notes_loop     = snap['notes_loop']
    notes_duration = prev_notes_dur
    cc_loop        = snap['cc_loop']
    cc_duration    = prev_cc_dur

    # stop CC playback if its creation is being undone
    if curr_cc_dur > 0.0 and prev_cc_dur == 0.0:
        cc_stop.set()

    # if notes buffer is gone, stop notes playback and return to idle
    if prev_notes_dur == 0.0:
        notes_stop.set()
        state = 'IDLE'
        print(f'  undo: notes buffer cleared → idle  ({len(undo_stack)} left)', flush=True)
    else:
        state = 'PLAYING'
        print(f'  undo: reverted  ({len(undo_stack)} left)', flush=True)

# ── Clear ─────────────────────────────────────────────────────────────────────

def do_clear():
    """Full clear. Call while holding lock."""
    global state, notes_loop, notes_duration, notes_overdub
    global cc_loop, cc_duration, cc_overdub, cc_dub_start
    global dub_type, _was_at_zero
    notes_stop.set()
    cc_stop.set()
    state          = 'IDLE'
    notes_loop     = []
    notes_duration = 0.0
    notes_overdub  = []
    cc_loop        = []
    cc_duration    = 0.0
    cc_overdub     = []
    cc_dub_start   = None
    dub_type       = None
    _was_at_zero   = False
    undo_stack.clear()
    print('■ CLEARED → idle', flush=True)

def _held_clear():
    """Called by timer after holding mod wheel at 0 for 8 beats."""
    with lock:
        do_clear()

# ── Playback ──────────────────────────────────────────────────────────────────

def _run_playback(get_events, get_duration, stop_evt, allow_send):
    while not stop_evt.is_set():
        with lock:
            events   = get_events()
            duration = get_duration()

        if not events or duration <= 0:
            time.sleep(0.002)
            continue

        loop_start = ts()
        for ev_time, msg in events:
            while True:
                remaining = (loop_start + ev_time) - ts()
                if remaining <= 0:
                    break
                if stop_evt.is_set():
                    return
                time.sleep(min(remaining, 0.001))
            if stop_evt.is_set():
                return
            with lock:
                if allow_send():
                    out.send(msg)

        loop_end = loop_start + duration
        while ts() < loop_end:
            if stop_evt.is_set():
                return
            time.sleep(0.001)

def start_notes_playback():
    notes_stop.clear()
    threading.Thread(target=_run_playback, daemon=True, kwargs=dict(
        get_events   = lambda: sorted(
            notes_loop + (notes_overdub if (state == 'DUBBING' and dub_type == 'notes') else []),
            key=lambda e: e[0],
        ),
        get_duration = lambda: notes_duration,
        stop_evt     = notes_stop,
        allow_send   = lambda: state in ('PLAYING', 'DUBBING'),
    )).start()

def start_cc_playback():
    cc_stop.clear()
    threading.Thread(target=_run_playback, daemon=True, kwargs=dict(
        get_events   = lambda: sorted(
            cc_loop + (cc_overdub if (state == 'DUBBING' and dub_type == 'cc') else []),
            key=lambda e: e[0],
        ),
        get_duration = lambda: cc_duration,
        stop_evt     = cc_stop,
        allow_send   = lambda: state in ('PLAYING', 'DUBBING'),
    )).start()

# ── Message handler ───────────────────────────────────────────────────────────

def on_message(msg):
    global state, rec_start_time, notes_duration, dub_type, cc_dub_start
    global notes_loop, notes_overdub, cc_loop, cc_overdub, cc_duration
    global current_bpm, _last_clock_ts, _zero_timer, _was_at_zero

    t = msg.type

    # --- BPM tracking from clock (not used for loop timing) ---
    if t == 'clock':
        now = ts()
        if _last_clock_ts is not None:
            dt = now - _last_clock_ts
            if 0.0005 < dt < 0.5:
                _clock_intervals.append(dt)
                if len(_clock_intervals) >= 4:
                    avg = sum(_clock_intervals) / len(_clock_intervals)
                    current_bpm = 60.0 / (avg * PPQN)
        _last_clock_ts = now
        return

    if t in ('active_sensing', 'sysex', 'start', 'stop', 'songpos'):
        return

    # --- mod wheel ---
    if t == 'control_change' and msg.control == 1:
        val = msg.value

        if val == 0:
            with lock:
                if not _was_at_zero:
                    _was_at_zero = True
                    hold_secs = 8 * 60.0 / current_bpm
                    print(f'  mod @ 0 — hold {hold_secs:.1f}s to clear, release to undo', flush=True)
            # start hold timer (outside lock)
            if _zero_timer:
                _zero_timer.cancel()
            _zero_timer = threading.Timer(8 * 60.0 / current_bpm, _held_clear)
            _zero_timer.start()

        else:
            # mod wheel left zero
            if _zero_timer:
                _zero_timer.cancel()
                _zero_timer = None

            with lock:
                if _was_at_zero:
                    _was_at_zero = False
                    # brief touch = undo
                    do_undo()
                    return

                # normal mod wheel logic
                if val == 127:
                    if state == 'IDLE':
                        state         = 'ARMED'
                        notes_loop    = []
                        notes_overdub = []
                        undo_stack.clear()
                        print('◉ ARMED → play a note to start recording', flush=True)
                    elif state == 'PLAYING':
                        state         = 'DUBBING'
                        dub_type      = None
                        notes_overdub = []
                        cc_overdub    = []
                        cc_dub_start  = None
                        print('● DUBBING → first note=note dub  |  first CC=CC dub', flush=True)

                else:  # 1-126
                    if state == 'RECORDING':
                        push_undo()  # snapshot: notes_duration=0, undo clears back to idle
                        notes_duration = ts() - rec_start_time
                        state          = 'PLAYING'
                        print(f'► PLAYING  ({len(notes_loop)} events, {notes_duration:.3f}s)', flush=True)
                        start_notes_playback()

                    elif state == 'ARMED':
                        state = 'IDLE'
                        print('  disarmed → idle', flush=True)

                    elif state == 'DUBBING':
                        if dub_type == 'notes':
                            if notes_overdub:
                                push_undo()
                                notes_loop.extend(notes_overdub)
                                notes_loop.sort(key=lambda e: e[0])
                                print(f'  note dub committed — {len(notes_overdub)} added, {len(notes_loop)} total', flush=True)
                            else:
                                print('  note dub exit (nothing added)', flush=True)
                            notes_overdub = []

                        elif dub_type == 'cc':
                            if cc_overdub and cc_dub_start is not None:
                                push_undo()
                                if cc_duration == 0.0:
                                    cc_loop     = list(cc_overdub)
                                    cc_duration = ts() - cc_dub_start
                                    print(f'  CC buffer created — {len(cc_loop)} events, {cc_duration:.3f}s', flush=True)
                                    start_cc_playback()
                                else:
                                    cc_loop.extend(cc_overdub)
                                    cc_loop.sort(key=lambda e: e[0])
                                    print(f'  CC dub merged — {len(cc_overdub)} added, {len(cc_loop)} total', flush=True)
                            else:
                                print('  CC dub exit (nothing added)', flush=True)
                            cc_overdub   = []
                            cc_dub_start = None

                        else:
                            print('  dub exit (nothing recorded)', flush=True)

                        dub_type = None
                        state    = 'PLAYING'
        return

    # --- notes ---
    if t in ('note_on', 'note_off'):
        with lock:
            if state == 'ARMED' and t == 'note_on' and msg.velocity > 0:
                state          = 'RECORDING'
                rec_start_time = ts()
                notes_loop     = [(0.0, msg)]
                print('● RECORDING notes', flush=True)

            elif state == 'RECORDING':
                notes_loop.append((ts() - rec_start_time, msg))

            elif state == 'DUBBING':
                if dub_type is None:
                    dub_type = 'notes'
                    print('  → note dub mode', flush=True)
                if dub_type == 'notes':
                    pos = (ts() - rec_start_time) % notes_duration
                    notes_overdub.append((pos, msg))
                else:
                    return  # CC dub active — ignore notes

            elif state == 'PLAYING':
                return  # ignore echoed notes

        out.send(msg)
        return

    # --- all other CC (knobs etc) ---
    if t == 'control_change':
        with lock:
            if state == 'DUBBING':
                if dub_type is None:
                    dub_type     = 'cc'
                    cc_dub_start = ts()
                    cc_overdub   = []
                    print('  → CC dub mode', flush=True)
                if dub_type == 'cc':
                    rel = ts() - cc_dub_start
                    if cc_duration > 0:
                        rel = rel % cc_duration  # wrap into loop on subsequent dubs
                    cc_overdub.append((rel, msg))
                else:
                    return  # note dub active — ignore CC

        out.send(msg)

# ── Main ──────────────────────────────────────────────────────────────────────

print(f'IN  : {IN_PORT}')
print(f'OUT : {OUT_PORT}\n')
print('MOD WHEEL:')
print('  full (127)       arm / enter dub')
print('  middle (1-126)   stop+loop / commit dub')
print('  0 brief          undo last change')
print('  0 hold 8 beats   clear everything\n')
print('DUB: first note → note dub  |  first CC → CC dub\n')

out = mido.open_output(OUT_PORT)

with mido.open_input(IN_PORT, callback=on_message) as port:
    print('Ready.\n')
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        if _zero_timer:
            _zero_timer.cancel()
        notes_stop.set()
        cc_stop.set()
        print('\nStopped.')
