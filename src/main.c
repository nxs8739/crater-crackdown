#include <gccore.h>
#include <ogc/lwp_watchdog.h>

#include "video_gc.h"
#include "input.h"
#include "render.h"
#include "game.h"
#include "audio.h"

#define SIMULATION_STEP (1.0 / 60.0)
#define MAX_FRAME_SECONDS 0.10
#define MAX_STEPS_PER_FRAME 6

/* Static game state: no per-frame heap allocations. */
static Game game;

int main(void)
{
    u64 last_time;
    double accumulator = 0.0;

    video_gc_init();
    input_init();
    render_init();
    game_init(&game, (uint32_t)gettime());
    audio_init();
    last_time = gettime();

    while (SYS_MainLoop()) {
        u64 now = gettime();
        double elapsed = (double)diff_usec(last_time, now) / 1000000.0;
        unsigned steps = 0;
        u32 held, pressed;
        GameInput controls = { 0, 0, 0, 0 };
        last_time = now;

        if (elapsed > MAX_FRAME_SECONDS) elapsed = MAX_FRAME_SECONDS;
        if (elapsed < 0.0) elapsed = 0.0;

        input_scan();
        held = input_held();
        pressed = input_pressed();

        /* START keeps the existing Phase 1 behavior: exit to the loader. */
        if (pressed & PAD_BUTTON_START) break;
        if (game.game_over && (pressed & PAD_BUTTON_X)) {
            game_init(&game, (uint32_t)now);
            audio_reset();
            accumulator = 0.0;
        }

        /* D-pad and left analog stick both rotate the ship. */
        {
            const int stick_x = PAD_StickX(0);
            const int deadzone = 24;
            controls.turn_left = ((held & PAD_BUTTON_LEFT) != 0) || (stick_x < -deadzone);
            controls.turn_right = ((held & PAD_BUTTON_RIGHT) != 0) || (stick_x > deadzone);
        }
        controls.thrust = (held & PAD_BUTTON_B) != 0;
        controls.fire = (held & PAD_BUTTON_A) != 0;

        /* Real elapsed time, independent of PAL 50/60 or NTSC VSync. */
        accumulator += elapsed;
        while (accumulator >= SIMULATION_STEP && steps < MAX_STEPS_PER_FRAME) {
            game_tick(&game, controls);
            audio_tick(&game, controls);
            accumulator -= SIMULATION_STEP;
            ++steps;
        }
        if (steps == MAX_STEPS_PER_FRAME && accumulator >= SIMULATION_STEP)
            accumulator = 0.0;

        /* Same known-good GX presentation path as the Phase 1 hardware test. */
        render_game(&game);
        video_gc_present();
    }
    audio_shutdown();
    return 0;
}
