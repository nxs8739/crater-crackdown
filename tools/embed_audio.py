#!/usr/bin/env python3
"""Rebuild embedded BE PCM arrays from the APPROVED v2 WAV files.

Run from anywhere; never alters the original WAV assets.  The GameCube
ASND VOICE_MONO_16BIT expects native (big-endian) signed 16-bit samples.
Heartbeat in-DOL samples receive subtle TV-speaker harmonics; the approved
WAV sources are not modified. Each buffer is padded/aligned for DSP reads.
"""
from pathlib import Path
import wave
import math
import struct

ROOT = Path(__file__).resolve().parent.parent
WAVDIR = ROOT / 'assets' / 'approved_wav'
SOURCE = ROOT / 'src' / 'audio_data.c'
HEADER = ROOT / 'src' / 'audio_data.h'
NAMES = (
    'asteroid_large', 'asteroid_medium', 'asteroid_small',
    'heartbeat_high', 'heartbeat_low', 'laser_shot',
    'ship_explosion', 'thrust_loop', 'ufo_hum_loop', 'ufo_shot',
)



def tv_speaker_heartbeat(pcm, name):
    """Add quiet overtones for small TV speakers, leaving the source WAV alone.

    The approved heartbeat fundamentals (~60/75 Hz) are below the useful
    response of many TV speakers. Keep the bass; add harmonics following a
    short soft-attack/decay envelope. Only the in-DOL PCM is adjusted.
    """
    base_hz = 61.0 if name == 'heartbeat_low' else 76.0
    samples = struct.unpack('<' + 'h' * (len(pcm) // 2), pcm)
    out = bytearray(len(pcm))
    for i, value in enumerate(samples):
        time_s = i / 32000.0
        envelope = (1.0 - math.exp(-time_s / 0.007)) * math.exp(-time_s / 0.072)
        third = 0.205 * math.sin(2.0 * math.pi * base_hz * 3.0 * time_s)
        fifth = 0.070 * math.sin(2.0 * math.pi * base_hz * 5.0 * time_s)
        # A small decaying attack helps speakers reveal each beat without
        # turning the sound into a click or changing the original rhythm.
        attack = 0.023 * math.sin(2.0 * math.pi * 720.0 * time_s) * math.exp(-time_s / 0.019)
        enhanced = value / 32768.0 + envelope * (third + fifth) + attack
        # Hard safety limit, normally never reached for these samples.
        enhanced = max(-1.0, min(32767.0 / 32768.0, enhanced))
        struct.pack_into('<h', out, i * 2, round(enhanced * 32768.0))
    return bytes(out)

def main():
    definitions = [
        '/* GENERATED FILE; do not hand-edit. */',
        '/* Embedded samples from approved BEANZ Asteroids Sound Pack v2. */',
        '/* Convert original little-endian WAV PCM to big-endian 16-bit. */',
        '#include "audio_data.h"',
        '',
    ]
    declarations = [
        '#ifndef BEANZ_AUDIO_DATA_H',
        '#define BEANZ_AUDIO_DATA_H',
        '/* Big-endian 32kHz mono 16-bit raw PCM, 32-byte aligned/padded. */',
        '',
    ]
    for name in NAMES:
        filename = WAVDIR / (name + '.wav')
        with wave.open(str(filename), 'rb') as wav:
            assert wav.getnchannels() == 1, filename
            assert wav.getsampwidth() == 2, filename
            assert wav.getframerate() == 32000, filename
            assert wav.getcomptype() == 'NONE', filename
            pcm = wav.readframes(wav.getnframes())
        assert len(pcm) % 2 == 0
        if name in ('heartbeat_low', 'heartbeat_high'):
            pcm = tv_speaker_heartbeat(pcm, name)
        # little endian -> big endian, preserving the approved audio waveform
        swapped = bytearray(len(pcm))
        swapped[0::2] = pcm[1::2]
        swapped[1::2] = pcm[0::2]
        sound_bytes = len(swapped)
        swapped.extend(b'\0' * (-sound_bytes % 32))
        assert len(swapped) % 32 == 0
        sym = 'sfx_' + name
        declarations.extend((
            f'extern const unsigned char {sym}[];',
            f'extern const unsigned int {sym}_bytes;',
        ))
        definitions.append(f'const unsigned char {sym}[] __attribute__((aligned(32))) = {{')
        for i in range(0, len(swapped), 16):
            definitions.append('    ' + ', '.join(f'0x{x:02x}' for x in swapped[i:i + 16]) + ',')
        definitions.extend(('};', f'const unsigned int {sym}_bytes = {sound_bytes}u;', ''))
        print(f'{name:<20} {sound_bytes:>6} bytes (original) {len(swapped):>6} bytes embedded')
    declarations.extend(('', '#endif', ''))
    SOURCE.write_text('\n'.join(definitions))
    HEADER.write_text('\n'.join(declarations))
    print('Wrote', SOURCE)


if __name__ == '__main__':
    main()
