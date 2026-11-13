#include "game.h"

#include <math.h>
#include <string.h>

#define TAU 6.28318530717958647692f
#define SHIP_RADIUS 15.0f
#define ROTATION_PER_STEP 0.060f
#define THRUST_ACCELERATION 210.0f
#define SHIP_MAX_SPEED 290.0f
#define SHIP_DRAG_PER_STEP 0.997f
#define BULLET_SPEED 400.0f
#define BULLET_LIFETIME 0.92f
#define SHOT_COOLDOWN 0.16f

static uint32_t random_next(Game *game)
{
    uint32_t value = game->rng;
    value ^= value << 13;
    value ^= value >> 17;
    value ^= value << 5;
    game->rng = value;
    return value;
}

static float random_unit(Game *game)
{
    return (float)(random_next(game) & 0x00ffffffu) / 16777216.0f;
}

static void wrap_position(GameVec2 *pos)
{
    if (pos->x < 0.0f) pos->x += GAME_WORLD_W;
    if (pos->x >= GAME_WORLD_W) pos->x -= GAME_WORLD_W;
    if (pos->y < 0.0f) pos->y += GAME_WORLD_H;
    if (pos->y >= GAME_WORLD_H) pos->y -= GAME_WORLD_H;
}

static float wrapped_distance_squared(GameVec2 a, GameVec2 b)
{
    float dx = fabsf(a.x - b.x);
    float dy = fabsf(a.y - b.y);
    if (dx > GAME_WORLD_W * 0.5f) dx = GAME_WORLD_W - dx;
    if (dy > GAME_WORLD_H * 0.5f) dy = GAME_WORLD_H - dy;
    return dx * dx + dy * dy;
}

float game_asteroid_radius(unsigned char size)
{
    if (size == ASTEROID_LARGE) return 34.0f;
    if (size == ASTEROID_MEDIUM) return 22.0f;
    return 12.0f;
}

int game_asteroid_count(const Game *game)
{
    int count = 0;
    int i;
    for (i = 0; i < GAME_MAX_ASTEROIDS; ++i)
        if (game->asteroids[i].active) ++count;
    return count;
}

static void reset_ship(Game *game)
{
    game->ship_position.x = GAME_WORLD_W * 0.5f;
    game->ship_position.y = GAME_WORLD_H * 0.5f;
    game->ship_velocity.x = 0.0f;
    game->ship_velocity.y = 0.0f;
    game->ship_angle = 0.0f;
    game->ship_alive = 1;
    game->invulnerable_time = 2.5f;
    game->respawn_time = 0.0f;
    game->shot_cooldown = 0.0f;
}

static void add_asteroid(Game *game, unsigned char size, GameVec2 pos,
                         float angle, float speed)
{
    int i;
    for (i = 0; i < GAME_MAX_ASTEROIDS; ++i) {
        GameAsteroid *asteroid = &game->asteroids[i];
        if (!asteroid->active) {
            asteroid->active = 1;
            asteroid->size = size;
            asteroid->position = pos;
            asteroid->velocity.x = cosf(angle) * speed;
            asteroid->velocity.y = sinf(angle) * speed;
            return;
        }
    }
}

static void start_wave(Game *game)
{
    int i;
    int number;
    game->wave += 1;
    if (game->wave >= 11) game->invulnerable_time = 4.0f;
    game->next_wave_time = -1.0f;
    number = game->wave;
    if (number > 10) number = 10;

    for (i = 0; i < number; ++i) {
        GameVec2 spawn;
        GameVec2 center = { GAME_WORLD_W * 0.5f, GAME_WORLD_H * 0.5f };
        float angle;
        float speed;
        int tries;
        /* Initial asteroids don't materialize on top of the player's ship. */
        for (tries = 0; tries < 30; ++tries) {
            spawn.x = random_unit(game) * GAME_WORLD_W;
            spawn.y = random_unit(game) * GAME_WORLD_H;
            if (wrapped_distance_squared(spawn, center) > 145.0f * 145.0f)
                break;
        }
        if (tries == 30) {
            spawn.x = 60.0f;
            spawn.y = 60.0f;
        }
        angle = random_unit(game) * TAU;
        speed = (36.0f + random_unit(game) * 24.0f) *
                (1.0f + fminf(0.055f * (game->wave - 1), 0.45f));
        add_asteroid(game, ASTEROID_LARGE, spawn, angle, speed);
    }
}

void game_init(Game *game, uint32_t seed)
{
    memset(game, 0, sizeof(*game));
    game->rng = seed ? seed : 0xa5b35711u;
    game->lives = 3;
    game->next_bonus_life = 10000;
    reset_ship(game);
    start_wave(game);
}

static void shoot(Game *game)
{
    int i;
    GameVec2 forward;
    forward.x = sinf(game->ship_angle);
    forward.y = -cosf(game->ship_angle);
    for (i = 0; i < GAME_MAX_BULLETS; ++i) {
        GameBullet *bullet = &game->bullets[i];
        if (!bullet->active) {
            bullet->active = 1;
            bullet->position.x = game->ship_position.x + forward.x * 21.0f;
            bullet->position.y = game->ship_position.y + forward.y * 21.0f;
            wrap_position(&bullet->position);
            bullet->velocity.x = game->ship_velocity.x * 0.25f + forward.x * BULLET_SPEED;
            bullet->velocity.y = game->ship_velocity.y * 0.25f + forward.y * BULLET_SPEED;
            bullet->lifetime = BULLET_LIFETIME;
            game->shot_cooldown = SHOT_COOLDOWN;
            game->sound_events |= GAME_SFX_SHOT;
            return;
        }
    }
}

static void destroy_asteroid(Game *game, int index)
{
    GameAsteroid asteroid = game->asteroids[index];
    float heading = atan2f(asteroid.velocity.y, asteroid.velocity.x);
    float speed = hypotf(asteroid.velocity.x, asteroid.velocity.y);
    unsigned char next_size;

    if (asteroid.size == ASTEROID_LARGE) game->sound_events |= GAME_SFX_LARGE_HIT;
    else if (asteroid.size == ASTEROID_MEDIUM) game->sound_events |= GAME_SFX_MEDIUM_HIT;
    else game->sound_events |= GAME_SFX_SMALL_HIT;
    game->asteroids[index].active = 0;
    if (asteroid.size == ASTEROID_LARGE) game->score += 20;
    else if (asteroid.size == ASTEROID_MEDIUM) game->score += 50;
    else game->score += 100;

    while (game->score >= game->next_bonus_life) {
        game->lives += 1;
        game->next_bonus_life += 10000;
    }

    if (asteroid.size == ASTEROID_SMALL) return;

    next_size = (unsigned char)(asteroid.size - 1);
    /* Identical crater graphics; child asteroid trajectories may vary. */
    add_asteroid(game, next_size, asteroid.position,
                 heading - 0.55f, speed * 1.32f);
    add_asteroid(game, next_size, asteroid.position,
                 heading + 0.55f, speed * 1.32f);
}

static void update_ship(Game *game, GameInput input)
{
    float speed_squared;
    float step = GAME_STEP_SECONDS;

    if (input.turn_left) game->ship_angle -= ROTATION_PER_STEP;
    if (input.turn_right) game->ship_angle += ROTATION_PER_STEP;
    if (game->ship_angle >= TAU) game->ship_angle -= TAU;
    if (game->ship_angle < 0.0f) game->ship_angle += TAU;

    if (input.thrust) {
        game->ship_velocity.x += sinf(game->ship_angle) * THRUST_ACCELERATION * step;
        game->ship_velocity.y -= cosf(game->ship_angle) * THRUST_ACCELERATION * step;
    }
    game->ship_velocity.x *= SHIP_DRAG_PER_STEP;
    game->ship_velocity.y *= SHIP_DRAG_PER_STEP;
    speed_squared = game->ship_velocity.x * game->ship_velocity.x +
                    game->ship_velocity.y * game->ship_velocity.y;
    if (speed_squared > SHIP_MAX_SPEED * SHIP_MAX_SPEED) {
        float scale = SHIP_MAX_SPEED / sqrtf(speed_squared);
        game->ship_velocity.x *= scale;
        game->ship_velocity.y *= scale;
    }
    game->ship_position.x += game->ship_velocity.x * step;
    game->ship_position.y += game->ship_velocity.y * step;
    wrap_position(&game->ship_position);

    if (game->shot_cooldown > 0.0f) game->shot_cooldown -= step;
    if (input.fire && game->shot_cooldown <= 0.0f) shoot(game);
}

void game_tick(Game *game, GameInput input)
{
    int i, j;
    const float step = GAME_STEP_SECONDS;

    game->sound_events = 0;
    if (game->game_over) return;

    if (game->invulnerable_time > 0.0f) game->invulnerable_time -= step;

    if (game->ship_alive) {
        update_ship(game, input);
    } else {
        game->respawn_time -= step;
        if (game->respawn_time <= 0.0f) reset_ship(game);
    }

    for (i = 0; i < GAME_MAX_ASTEROIDS; ++i) {
        GameAsteroid *asteroid = &game->asteroids[i];
        if (!asteroid->active) continue;
        asteroid->position.x += asteroid->velocity.x * step;
        asteroid->position.y += asteroid->velocity.y * step;
        wrap_position(&asteroid->position);
    }

    for (i = 0; i < GAME_MAX_BULLETS; ++i) {
        GameBullet *bullet = &game->bullets[i];
        if (!bullet->active) continue;
        bullet->position.x += bullet->velocity.x * step;
        bullet->position.y += bullet->velocity.y * step;
        wrap_position(&bullet->position);
        bullet->lifetime -= step;
        if (bullet->lifetime <= 0.0f) {
            bullet->active = 0;
            continue;
        }
        for (j = 0; j < GAME_MAX_ASTEROIDS; ++j) {
            GameAsteroid *asteroid = &game->asteroids[j];
            float radius;
            if (!asteroid->active) continue;
            radius = game_asteroid_radius(asteroid->size) + 2.5f;
            if (wrapped_distance_squared(bullet->position, asteroid->position)
                <= radius * radius) {
                bullet->active = 0;
                destroy_asteroid(game, j);
                break;
            }
        }
    }

    if (game->ship_alive && game->invulnerable_time <= 0.0f) {
        for (i = 0; i < GAME_MAX_ASTEROIDS; ++i) {
            GameAsteroid *asteroid = &game->asteroids[i];
            float radius;
            if (!asteroid->active) continue;
            radius = game_asteroid_radius(asteroid->size) + SHIP_RADIUS;
            if (wrapped_distance_squared(game->ship_position, asteroid->position)
                <= radius * radius) {
                game->lives--;
                game->sound_events |= GAME_SFX_SHIP_EXPLODE;
                game->ship_alive = 0;
                game->ship_velocity.x = 0.0f;
                game->ship_velocity.y = 0.0f;
                if (game->lives <= 0) game->game_over = 1;
                else game->respawn_time = 1.1f;
                break;
            }
        }
    }

    if (game_asteroid_count(game) == 0) {
        if (game->next_wave_time < 0.0f) game->next_wave_time = 0.9f;
        else {
            game->next_wave_time -= step;
            if (game->next_wave_time <= 0.0f) start_wave(game);
        }
    } else {
        game->next_wave_time = -1.0f;
    }
}
