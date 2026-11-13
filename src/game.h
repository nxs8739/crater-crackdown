#ifndef BEANZ_GAME_H
#define BEANZ_GAME_H

#include <stdint.h>

/* Portable game-state module: deliberately independent of libogc/GX. */
#define GAME_MAX_ASTEROIDS 64
#define GAME_MAX_BULLETS 6
#define GAME_WORLD_W 640.0f
#define GAME_WORLD_H 480.0f
#define GAME_STEP_SECONDS (1.0f / 60.0f)

typedef enum {
    ASTEROID_SMALL = 0,
    ASTEROID_MEDIUM = 1,
    ASTEROID_LARGE = 2
} AsteroidSize;

typedef struct { float x, y; } GameVec2;

typedef struct {
    unsigned char active;
    unsigned char size;
    GameVec2 position;
    GameVec2 velocity;
} GameAsteroid;

typedef struct {
    unsigned char active;
    GameVec2 position;
    GameVec2 velocity;
    float lifetime;
} GameBullet;

typedef struct {
    unsigned char turn_left;
    unsigned char turn_right;
    unsigned char thrust;
    unsigned char fire;
} GameInput;

/* Event bits are emitted by each simulation tick, not persistent state. */
enum {
    GAME_SFX_SHOT          = 1u << 0,
    GAME_SFX_LARGE_HIT     = 1u << 1,
    GAME_SFX_MEDIUM_HIT    = 1u << 2,
    GAME_SFX_SMALL_HIT     = 1u << 3,
    GAME_SFX_SHIP_EXPLODE  = 1u << 4
};

typedef struct {
    GameVec2 ship_position;
    GameVec2 ship_velocity;
    float ship_angle;
    float invulnerable_time;
    float respawn_time;
    float shot_cooldown;
    float next_wave_time;
    int score;
    int lives;
    int wave;
    int next_bonus_life;
    unsigned char ship_alive;
    unsigned char game_over;
    uint32_t rng;
    uint32_t sound_events; /* cleared and updated every game_tick */
    GameAsteroid asteroids[GAME_MAX_ASTEROIDS];
    GameBullet bullets[GAME_MAX_BULLETS];
} Game;

void game_init(Game *game, uint32_t seed);
void game_tick(Game *game, GameInput input);
float game_asteroid_radius(unsigned char size);
int game_asteroid_count(const Game *game);

#endif
