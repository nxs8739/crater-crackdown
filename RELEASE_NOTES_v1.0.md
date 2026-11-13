# Crater Crackdown — v1.0 Release Notes

**Initial public release — Nintendo GameCube homebrew**

Crater Crackdown is an independent, Asteroids-inspired arcade shooter built in C with devkitPPC r51 and **libogc 3.2.1**. It runs as a standalone `.dol` executable, with graphics and synthesized audio embedded into the program.

## Features

- Arcade-style ship rotation, momentum-based thrust, firing, screen wrapping, and asteroid collision.
- Three asteroid sizes with unique fixed crater layouts. Large asteroids split into two medium, which split into two small.
- Three starting lives, a new life every 10,000 points, current-run scoring, and wave progression.
- One starting large asteroid on wave 1, increasing by one per wave until the ten-asteroid cap at wave 10.
- Asteroid-speed difficulty increases through wave 10, then remains capped for later waves.
- **Wave 11 onward:** four seconds of flashing invulnerability when each new wave begins. The existing timer is set to exactly four seconds; it is not stacked with remaining protection.
- Normal 2.5-second flashing invulnerability after respawning from a lost life.
- Embedded original synthesized laser, asteroid-break, explosion, thrust, and heartbeat sound effects.
- Fixed 60 Hz gameplay simulation and refresh-aware video presentation.
- D-pad or analog-stick rotation; B thrust, A fire, X restart on Game Over, START exit.

## Requirements

- A homebrew-capable Nintendo GameCube with a compatible DOL loader such as Swiss.
- **No extra SD card game assets, save files, network adapter, or Memory Card writes required.**

## Installation

1. Download **`Crater_Crackdown_v1.0.dol`** from this GitHub Release (once you have built and uploaded it).
2. Copy the DOL to your SD card or other media accessible to your GameCube loader.
3. Launch it with Swiss.

## Validation status

- Tested during gameplay on a real GameCube DOL-001 using Swiss, including the updated early-wave progression.
- Desktop gameplay/audio tests, a **100,000-tick stress test**, and a separate host-side wave-11–13 protection check passed.
- **Not yet verified on physical hardware:** the new wave-11+ four-second protection (the developer has not reached wave 11 in a live test).
- PAL hardware was **not** independently tested.

## Deliberate v1.0 scope

No UFO enemies, high-score saving, networking, external runtime assets, or SD/Memory Card writing. Two UFO-themed source sound effects exist in the project but are unused.

## Legal and licensing

Original project work is MIT-licensed. This is an independent, unofficial title inspired by classic arcade gameplay. **Not affiliated with Atari or Nintendo.** See `LICENSE` and `LEGAL.md` in the source repository for details.

**Note for the publisher:** These release notes assume you have freshly built the final wave-11+ source and uploaded that new DOL. Do not upload the older executable carried in the original development archive.
