#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "game.h"

static Game game;

static void clear_asteroids(void)
{
    int i;
    for (i = 0; i < GAME_MAX_ASTEROIDS; ++i) game.asteroids[i].active = 0;
}

static void clear_bullets(void)
{
    int i;
    for (i = 0; i < GAME_MAX_BULLETS; ++i) game.bullets[i].active = 0;
}

static void forced_hit(unsigned char size, int expected_score, int expected_count)
{
    GameInput idle = {0};
    game_init(&game, 123u);
    clear_asteroids();
    clear_bullets();
    game.asteroids[0].active = 1;
    game.asteroids[0].size = size;
    game.asteroids[0].position.x = 100.0f;
    game.asteroids[0].position.y = 100.0f;
    game.asteroids[0].velocity.x = 0.0f;
    game.asteroids[0].velocity.y = 0.0f;
    game.bullets[0].active = 1;
    game.bullets[0].position.x = 100.0f;
    game.bullets[0].position.y = 100.0f;
    game.bullets[0].velocity.x = 0.0f;
    game.bullets[0].velocity.y = 0.0f;
    game.bullets[0].lifetime = 1.0f;
    game_tick(&game, idle);
    assert(game.score == expected_score);
    assert(game_asteroid_count(&game) == expected_count);
    assert(!game.bullets[0].active);
}

int main(void)
{
    GameInput controls = {0};
    float original_x, original_y;
    int i;
    game_init(&game, 123u);
    assert(game.lives == 3);
    assert(game.wave == 1);
    assert(game_asteroid_count(&game) == 1);
    assert(game.score == 0);
    assert(game_asteroid_radius(ASTEROID_LARGE) > game_asteroid_radius(ASTEROID_MEDIUM));

    /* At heading zero, UP thrust moves ship toward negative Y. */
    clear_asteroids();
    original_x = game.ship_position.x;
    original_y = game.ship_position.y;
    controls.thrust = 1;
    for (i = 0; i < 30; ++i) game_tick(&game, controls);
    assert(fabsf(game.ship_position.x - original_x) < 0.01f);
    assert(game.ship_position.y < original_y);
    assert(game.ship_velocity.y < 0.0f);

    /* Bullets are created on fire and have time-to-live. */
    controls.thrust = 0;
    controls.fire = 1;
    game_tick(&game, controls);
    {
        int active = 0;
        for (i = 0; i < GAME_MAX_BULLETS; ++i)
            if (game.bullets[i].active) ++active;
        assert(active >= 1);
    }

    forced_hit(ASTEROID_LARGE, 20, 2);
    forced_hit(ASTEROID_MEDIUM, 50, 2);
    forced_hit(ASTEROID_SMALL, 100, 0);

    /* First bonus life is awarded at 10,000 total points. */
    game_init(&game, 123u);
    clear_asteroids();
    game.score = 9980;
    game.asteroids[0].active = 1;
    game.asteroids[0].size = ASTEROID_LARGE;
    game.asteroids[0].position.x = 100;
    game.asteroids[0].position.y = 100;
    game.bullets[0].active = 1;
    game.bullets[0].lifetime = 1.0f;
    game.bullets[0].position.x = 100;
    game.bullets[0].position.y = 100;
    controls = (GameInput){0};
    game_tick(&game, controls);
    assert(game.score == 10000);
    assert(game.lives == 4);
    assert(game.next_bonus_life == 20000);

    /* Wrapped positions and seam-aware collisions. */
    game_init(&game, 123u);
    clear_asteroids();
    game.ship_position.x = 639.5f;
    game.ship_position.y = 200;
    game.ship_velocity.x = 200;
    game.ship_velocity.y = 0;
    game_tick(&game, (GameInput){0});
    assert(game.ship_position.x < 10);
    assert(game.ship_position.x >= 0);

    /* Ship loses a life on contact after respawn protection expires. */
    game_init(&game, 123u);
    clear_asteroids();
    game.invulnerable_time = 0.0f;
    game.asteroids[0].active = 1;
    game.asteroids[0].size = ASTEROID_SMALL;
    game.asteroids[0].position = game.ship_position;
    game.asteroids[0].velocity.x = game.asteroids[0].velocity.y = 0;
    game_tick(&game, (GameInput){0});
    assert(game.lives == 2);
    assert(!game.ship_alive);
    assert(!game.game_over);
    printf("PASS: initial wave, thrust, shooting, 3 asteroid split sizes, scoring, bonus life, wrap, collision\n");
    return 0;
}
