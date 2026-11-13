/* Compiles audio.c against a host stub.  This does NOT emulate GameCube DSP. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <asndlib.h>
#include "game.h"
#include "audio.h"
#include "audio_data.h"

static int state[16], calls[16];
static const void *last_data[16];
static int last_pitch[16];
static int init_count, end_count, unpaused;

void ASND_Init(void) { ++init_count; memset(state, 0, sizeof(state)); }
void ASND_End(void) { ++end_count; }
void ASND_Pause(s32 paused) { if (paused == 0) unpaused = 1; }
s32 ASND_SetVoice(s32 voice, s32 format, s32 pitch, s32 delay,
                  void *data, s32 size, s32 left, s32 right,
                  void (*callback)(s32 voice))
{
    assert(voice >= 0 && voice < 16);
    assert(format == VOICE_MONO_16BIT);
    assert(pitch == 32000 && delay == 0);
    assert(data != NULL && size > 0 && size % 32 == 0);
    assert(((uintptr_t)data & 31u) == 0);
    assert(left > 0 && right > 0 && callback == NULL);
    state[voice] = SND_WORKING;
    calls[voice]++;
    last_pitch[voice] = pitch;
    last_data[voice] = data;
    return SND_OK;
}
s32 ASND_StopVoice(s32 voice) { assert(voice >= 0 && voice < 16); state[voice] = SND_UNUSED; return SND_OK; }
s32 ASND_StatusVoice(s32 voice) { assert(voice >= 0 && voice < 16); return state[voice]; }

int main(void)
{
    Game game;
    GameInput input = {0};
    int i;
    game_init(&game, 1);
    audio_init();
    assert(init_count == 1 && unpaused);

    /* Testing gameplay shooting event dispatch; no runtime file accesses. */
    game.sound_events = GAME_SFX_SHOT;
    audio_tick(&game, input);
    assert(calls[4] == 1);
    assert(last_data[4] == sfx_laser_shot && last_pitch[4] == 32000);

    game.sound_events = GAME_SFX_LARGE_HIT | GAME_SFX_MEDIUM_HIT | GAME_SFX_SMALL_HIT;
    audio_tick(&game, input);
    assert(last_data[5] == sfx_asteroid_large);
    assert(last_data[6] == sfx_asteroid_medium);
    assert(last_data[7] == sfx_asteroid_small);

    game.sound_events = 0;
    input.thrust = 1;
    audio_tick(&game, input);
    assert(last_data[2] == sfx_thrust_loop);
    input.thrust = 0;
    audio_tick(&game, input);
    assert(state[2] == SND_UNUSED);

    game.sound_events = GAME_SFX_SHIP_EXPLODE;
    game.ship_alive = 0;
    game.game_over = 1;
    audio_tick(&game, input);
    assert(last_data[3] == sfx_ship_explosion);

    game_init(&game, 2);
    audio_reset();
    assert(state[3] == SND_UNUSED);
    game.sound_events = 0;
    for (i = 0; i < 60; ++i) {
        /* Simulate natural sound completion between beats. */
        if (i % 6 == 0) state[1] = SND_UNUSED;
        audio_tick(&game, input);
    }
    assert(calls[1] >= 1);
    assert(last_data[1] == sfx_heartbeat_low || last_data[1] == sfx_heartbeat_high);

    audio_shutdown();
    assert(end_count == 1);
    printf("PASS: audio initialization, 32000Hz mono PCM dispatch, sound events, thrust, death, heartbeat, shutdown\n");
    return 0;
}
