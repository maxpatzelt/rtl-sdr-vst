#!/usr/bin/env python3
"""
MIDI Monitor — prints every incoming message with timestamp.
Usage: python3 monitor.py
"""

import mido
import time

PORT_NAME = 'UM-ONE:UM-ONE MIDI 1 24:0'

def format_msg(msg):
    t = msg.type
    if t == 'clock':
        return None  # too noisy, suppress
    if t == 'active_sensing':
        return None  # suppress
    ts = time.strftime('%H:%M:%S')
    return f'[{ts}]  {msg}'

print(f'Opening: {PORT_NAME}')
print('Listening... (Ctrl+C to stop)\n')

with mido.open_input(PORT_NAME) as port:
    for msg in port:
        line = format_msg(msg)
        if line:
            print(line, flush=True)
