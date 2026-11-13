#ifndef BEANZ_AUDIO_H
#define BEANZ_AUDIO_H

#include "game.h"

/* Uses libogc ASND DSP mixer, no runtime files or memory-card access. */
void audio_init(void);
void audio_reset(void);
void audio_tick(const Game *game, GameInput controls);
void audio_shutdown(void);

#endif
