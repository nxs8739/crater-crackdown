# Crater Crackdown — v1.2

Playfield Boundary Visibility Update

Version 1.2 introduces a permanent white border around the playable area, making the screen-wrapping boundaries easier to identify on television displays.

## Changes

- Added a thin, solid white border on all four sides of the playfield, visible during gameplay and on the Game Over screen.
- Inset the rendered playfield by approximately 5% on each side of the active video mode to reduce the risk of television overscan clipping the border.
- Aligned the visible border with the existing screen-wrapping boundaries, with geometry clipped at the playfield edges.
- The border is always rendered, regardless of television aspect ratio.
- Preserved the existing 640 × 480 logical game world and HUD layout.

## Gameplay

No changes to gameplay mechanics, controls, physics, collision rules, asteroid behavior, difficulty progression, audio, scoring, or invulnerability. All gameplay functionality remains identical to v1.1. Asteroids continue to pass through one another.

## Validation

Tested by the developer on a real GameCube connected to a widescreen television. All four borders were visible, and gameplay remained normal through wave 6. Host-side gameplay, mocked audio, and 100,000-tick simulation tests passed. PAL hardware and other television configurations have not been independently verified with this update.
