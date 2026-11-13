#!/bin/sh
# Portable C logic and mocked audio tests (no GameCube toolchain needed).
set -eu
cd "$(dirname "$0")/.."
mkdir -p .host-test-build
cc -std=c99 -O2 -Wall -Wextra -Werror -Isrc src/game.c tests/test_game.c -lm -o .host-test-build/test_game
.host-test-build/test_game
cc -std=c99 -O2 -Wall -Wextra -Werror -Isrc -Itests/mocks \
    src/game.c src/audio.c src/audio_data.c tests/test_audio.c -lm -o .host-test-build/test_audio
.host-test-build/test_audio
cc -std=c99 -O2 -Wall -Wextra -Werror -Isrc src/game.c tests/stress_game.c -lm -o .host-test-build/stress_game
.host-test-build/stress_game
