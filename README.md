# midi-looper

A MIDI looper for Raspberry Pi controlled entirely by the mod wheel on a MicroKorg. No screen, no buttons, no fuss. Plug it in, it boots, it works.

Built for live use — record a loop, layer notes on top, automate knobs in a separate buffer, undo mistakes, clear and start again. All from one knob.

---

## what it does

Two independent loop buffers running simultaneously:

- **notes buffer** — whatever you play on the keys
- **CC buffer** — whatever knobs you tweak

Both have their own loop length and loop independently of each other.

---

## mod wheel

| position | what happens |
|----------|-------------|
| **full (127)** | arm to record (idle) / enter dub mode (playing) |
| **middle (1–126)** | stop recording + start loop / commit dub |
| **0, brief** | undo last thing you added |
| **0, hold 8 beats** | clear everything, back to idle |

---

## workflow

**record a loop**
1. mod wheel to full → armed
2. play something
3. mod wheel to middle → locks in loop length, starts playing

**dub notes**
1. mod wheel to full → dub mode
2. play a note → you're in note dub
3. mod wheel to middle → commits, layers onto existing loop

**dub CC / automation**
1. mod wheel to full → dub mode
2. tweak a knob → you're in CC dub mode, separate buffer, own length
3. mod wheel to middle → commits

**undo**
- brief 0 on the mod wheel undoes the last committed thing
- keep undoing to walk back through your history
- undo past the original recording → back to idle, fresh start

**clear**
- hold mod wheel at 0 for 8 beats → everything stops, blank slate

---

## hardware

- Raspberry Pi 3
- Roland UM-ONE USB MIDI interface
- MicroKorg (clock set to Internal)

## setup

```bash
pip install mido python-rtmidi
python3 looper.py
```

Runs as a systemd service on boot. Check logs with `journalctl -u midi-looper -f`.
