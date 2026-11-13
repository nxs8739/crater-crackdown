#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include "game.h"
int main(void) {
    Game g;
    int frame, i;
    game_init(&g, 987654321u);
    for (frame = 0; frame < 100000; ++frame) {
        GameInput input = {0};
        if (g.game_over) game_init(&g, (uint32_t)frame + 1u);
        input.turn_left = (frame % 91) < 11;
        input.turn_right = (frame % 127) < 15;
        input.thrust = (frame % 113) < 59;
        input.fire = (frame % 7) < 4;
        game_tick(&g, input);
        assert(g.ship_position.x >= 0 && g.ship_position.x < GAME_WORLD_W);
        assert(g.ship_position.y >= 0 && g.ship_position.y < GAME_WORLD_H);
        assert(isfinite(g.ship_position.x) && isfinite(g.ship_position.y));
        assert(game_asteroid_count(&g) <= GAME_MAX_ASTEROIDS);
        for (i = 0; i < GAME_MAX_ASTEROIDS; ++i)
            if (g.asteroids[i].active) {
                assert(g.asteroids[i].position.x >= 0 && g.asteroids[i].position.x < GAME_WORLD_W);
                assert(g.asteroids[i].position.y >= 0 && g.asteroids[i].position.y < GAME_WORLD_H);
                assert(g.asteroids[i].size <= ASTEROID_LARGE);
            }
        for (i = 0; i < GAME_MAX_BULLETS; ++i)
            if (g.bullets[i].active) {
                assert(g.bullets[i].position.x >= 0 && g.bullets[i].position.x < GAME_WORLD_W);
                assert(g.bullets[i].position.y >= 0 && g.bullets[i].position.y < GAME_WORLD_H);
            }
    }
    puts("PASS: 100,000 consecutive gameplay ticks with no invalid positions or states");
    return 0;
}
