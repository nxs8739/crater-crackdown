# GitHub Publishing Checklist — Crater Crackdown v1.0

This checklist is for the project publisher. **It is not a gameplay change.**

## 1. Local build and smoke test

1. Extract the GitHub-ready source archive into its own directory.
2. With devkitPPC r51 and libogc 3.2.1 available, run `make clean && make`.
3. Run `tests/run_host_tests.sh` (desktop-side logic/mocked-audio tests).
4. Confirm that the newly built `beanz_asteroids.dol` is generated after the build.
5. Boot that new DOL through Swiss and check startup, controls, sound, scoring, death/respawn, and exit.
6. The wave-11+ four-second invulnerability passed a separate host-side check, but **physical wave-11 observation remains pending**. Do not mark it tested on hardware until witnessed.

## 2. Prepare the GitHub repository

- Suggested repository name: `Crater-Crackdown`.
- Upload the contents of the clean source archive **at the repository root** (not nested under an extra parent folder).
- Verify the root contains `README.md`, `LICENSE`, `LEGAL.md`, `RELEASE_NOTES_v1.0.md`, `Makefile`, `src/`, `tests/`, `tools/`, and `assets/`.
- Keep build objects, host-test outputs, old `.elf` files, and older `.dol` files **out of the source repository**. The existing `.gitignore` excludes them.
- If desired, add the actual gameplay images and icon to the repository in a separate folder. Keep the Dolphin Depot artwork folder separate if preferred.

## 3. Create GitHub Release v1.0

1. On your repository page, open **Releases** → **Draft a new release**.
2. Create tag **`v1.0`** targeting the final uploaded source.
3. Set release title **`Crater Crackdown v1.0`**.
4. Paste the text of `RELEASE_NOTES_v1.0.md` into the release description. Remove the final publisher reminder paragraph before publishing.
5. Upload a **freshly built and hardware-smoke-tested** DOL, optionally copied to the filename `Crater_Crackdown_v1.0.dol`.
6. Publish the release only after you verify the downloadable release asset is the new build, **not the stale binary from the old ZIP**.

GitHub automatically provides source-code archives for tags. You generally do not need to attach another source ZIP if the repository already has all source files.

## 4. Dolphin Depot (separate step)

- Prepare `hero.png`, `banner.png`, `screenshot0.png`, and `screenshot1.png` in the dimensions expected by Dolphin Depot.
- Use the official GitHub release URL for the DOL distribution entry, following Dolphin Depot's current manifest/schema requirements.
- Confirm with the maintainer whether screenshots reconstructed from gameplay video rather than direct captures are acceptable; do not call them direct emulator captures.

## 5. Preserve a private archive

Retain the user's original source ZIP and the known-good DOL separately from the public release. Keep at least two backups.
