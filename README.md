# Crater Crackdown

**An original, Asteroids-inspired arcade shooter for the Nintendo GameCube.** Built in C using **devkitPPC** and the original **libogc 3.2.1** (not libogc2). Developed by **BEANZ** with AI-assisted implementation and review.

Pilot a triangle ship, dodge drifting rocks, and blast large asteroids into smaller, faster fragments. Crater Crackdown is a self-contained GameCube `.dol` with embedded graphics and sound: no runtime asset folder, network connection, save file, or Memory Card access is required.

## Playing

Load the release `.dol` using [Swiss](https://github.com/emukidid/swiss-gc) on a homebrew-capable GameCube. The application starts directly in gameplay.

| GameCube control | Action |
| --- | --- |
| D-pad Left / Right | Rotate ship |
| Left analog stick Left / Right | Rotate ship |
| **B** | Thrust |
| **A** | Fire (hold to keep shooting) |
| **X** | Restart after Game Over |
| **START** | Exit the application / return to loader |

> D-pad Up does **not** activate thrust. Use B. The ship retains momentum when thrust is released.

## Gameplay

- **Three starting lives**; earn another life every **10,000 points**.
- **Asteroid splitting:** large → two medium → four small. Each size has its own fixed crater arrangement (large: 3; medium: 2; small: 1).
- **Scoring:** large = 20 points, medium = 50 points, small = 100 points.
- **Wave progression:** wave 1 begins with one large asteroid; each successive wave adds one, up to **ten starting large asteroids from wave 10 onward**.
- **Speed difficulty:** initial asteroid speed increases through wave 10; the multiplier is capped thereafter.
- **Respawn protection:** after losing a life, the ship reappears with **2.5 seconds of flashing invulnerability** (unless it was the final life).
- **High-wave protection:** beginning at **wave 11**, each new wave resets the ship's invulnerability timer to **exactly 4.0 seconds**. The ship flashes using the same effect as its normal respawn. Waves 1–10 have no added wave-start protection.
- **Screen wrapping** for ship, bullets, and asteroids; a current-run score only, with no persistent high-score table.
- **Original synthesized sound effects** for laser shots, asteroid destruction, ship explosion, thrust, and alternating heartbeat. The heartbeat tempo follows how many asteroid fragments remain.
- **No UFO enemies** in v1.0. Two unused UFO-themed development sounds are embedded but not triggered in gameplay.

## Video and hardware

- Logical playfield: **640 × 480**.
- Fixed **60 Hz simulation**, with display presentation adapted to the console's video refresh (NTSC/PAL).
- Tested through repeated gameplay on an original **DOL-001 GameCube**, running via **PicoBoot / Swiss**.
- **PAL console behavior has not been independently hardware-tested.**
- The new **wave-11+ four-second invulnerability rule** passed a separate host-side wave-transition check, but has **not yet been directly observed on real hardware at wave 11**. Earlier-wave gameplay has been tested on a real GameCube.

## Building from source

Requirements: the **devkitPro GameCube toolchain**, **devkitPPC r51**, and **libogc 3.2.1** (including ASND). Build using the original libogc, **not libogc2**.

From the project root:

```sh
make clean
make
```

With the existing Makefile, the output names are:

- `beanz_asteroids.dol` — executable to run on GameCube.
- `beanz_asteroids.elf` — development build output.

The legacy output basename is preserved intentionally to avoid changing or risking the frozen game build during release preparation.

The `assets/approved_wav/` WAV sources are included to document the original synthesized effects, but the game does not read these at runtime. Embedded sound arrays in `src/audio_data.c/.h` are already generated, so no audio conversion step is required for an ordinary build. `tools/embed_audio.py` is a development tool, not part of normal gameplay.

## Testing on a desktop

The project includes host-side tests for core gameplay, mock ASND audio behavior, and a long-running simulation stress test. On Linux with a standard C compiler:

```sh
tests/run_host_tests.sh
```

These tests are **not** equivalent to testing the GPU, audio hardware, or gameplay on a real GameCube.

## Project layout

| Path | Contents |
| --- | --- |
| `src/` | Core C game logic, input, GX renderer, video, audio, embedded sound data |
| `assets/approved_wav/` | Original source WAV files for the sound pack |
| `tests/` | Portable gameplay tests, audio mocks, stress test |
| `tools/embed_audio.py` | Development utility for generating embedded sample data |
| `Makefile` | GameCube toolchain build |
| `LICENSE` | MIT license for original project work |
| `LEGAL.md` | Attribution, independence, and third-party notes |
| `RELEASE_NOTES_v1.0.md` | Release notes for GitHub |

## License and acknowledgements

Original Crater Crackdown project code and originally created assets are distributed under the **MIT License** (see [`LICENSE`](LICENSE)). Third-party toolchains, libraries, and utilities retain their own licenses; see [`LEGAL.md`](LEGAL.md).

This is an **independent, unofficial, Asteroids-inspired homebrew project**, **not affiliated with, endorsed by, or sponsored by Atari or Nintendo**. No original Atari ROM, code, graphics, or audio recordings are included.

Development was directed and hardware-tested by **BEANZ**, with AI assistance (ChatGPT and Qwen) for implementation and technical review. Thanks to the GameCube homebrew and devkitPro communities for the development ecosystem.
