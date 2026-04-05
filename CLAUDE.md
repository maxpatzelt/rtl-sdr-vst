# midi-looper

MIDI looper running on a Raspberry Pi 3, controlled entirely via the MicroKorg mod wheel.
No screen, no keyboard — standalone headless operation.

## Hardware
- **Device**: Raspberry Pi 3
- **SSH**: `ssh maxpi@192.168.0.23` (pass: Atreyu123)
- **MIDI interface**: Roland UM-ONE USB MIDI cable
- **Synth**: MicroKorg (MIDI IN + OUT via UM-ONE)
- **MicroKorg clock**: set to Internal — Pi receives F8 clock ticks

## MIDI Ports (on Pi)
- IN/OUT: `UM-ONE:UM-ONE MIDI 1 24:0`
- Defined at top of `looper.py` as `IN_PORT` / `OUT_PORT`

## Files
- `looper.py` — main looper, the only file needed at runtime
- `monitor.py` — debug tool: prints all incoming MIDI messages (suppress clock/active sensing)
- `requirements.txt` — `mido>=1.3.0`, `python-rtmidi>=1.5.0`

## Running
```bash
python3 ~/midi-looper/looper.py
```
Runs as a systemd service (`midi-looper.service`) on boot — no SSH needed.

## Mod Wheel Controls (CC#01)
| Wheel position | Action |
|----------------|--------|
| Full (127) | IDLE: arm to record \| PLAYING: enter dub mode |
| Middle (1-126) | RECORDING: stop, fix loop length, start playing \| DUBBING: commit dub |
| 0 brief | Undo last committed change. DUBBING: cancel dub without committing |
| 0 held 8 beats | Clear everything, stop all playback, back to IDLE |

## State Machine
```
IDLE → (mod wheel full) → ARMED → (first note) → RECORDING → (mod wheel mid) → PLAYING
PLAYING → (mod wheel full) → DUBBING → (mod wheel mid) → PLAYING
any state → (mod wheel 0 held 8 beats) → IDLE
```

## Two Independent Buffers
**Notes buffer**
- Length fixed when you move mod wheel from RECORDING to PLAYING
- Dub: enter dub, play a note first → note dub mode. Layers onto existing notes loop. Length never changes.

**CC buffer**
- Created the first time you enter dub mode and tweak a CC knob (not a note)
- Length fixed when you exit that first CC dub session
- Subsequent CC dubs add onto the CC buffer (same length, forever)
- Loops independently of the notes buffer

In dub mode, the first message (note vs CC) determines which buffer you're writing to.
Once determined, the other type is ignored for that session.

## Undo
- Each commit (note dub, CC dub, or initial recording) pushes a snapshot to the undo stack
- Brief mod wheel to 0 pops and restores the previous snapshot
- Undoing past the initial recording clears notes_duration → back to IDLE, ready for fresh session
- Undoing past first CC dub clears cc_duration → CC buffer gone, ready for fresh CC dub

## BPM / Clock
- MIDI clock (F8) is tracked passively for BPM only
- Loop timing is purely wall-clock (time.perf_counter) — not quantized to MIDI clock
- BPM is used solely to calculate the 8-beat hold duration for full clear
- Falls back to 120 BPM if no clock detected

## Systemd Service
```
/etc/systemd/system/midi-looper.service
```
Starts on boot, restarts on crash. Logs via journald:
```bash
journalctl -u midi-looper -f
```

## Dependencies
```bash
pip3 install mido python-rtmidi
```
Already installed in `~/midi-looper/venv` on the Pi.
