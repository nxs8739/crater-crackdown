#include <gccore.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>

#include "game.h"
#include "render.h"

#define CIRCLE_SEGMENTS 24
#define SHIP_RADIUS 20.0f

/* One common outline-free circle body, three fixed crater patterns. */
static float circle_cos[CIRCLE_SEGMENTS + 1];
static float circle_sin[CIRCLE_SEGMENTS + 1];
static const GXColor COLOR_BODY = { 224, 224, 224, 255 };
static const GXColor COLOR_CRATER = { 24, 24, 24, 255 };
static const GXColor ASTEROIDS_WHITE = { 255, 255, 255, 255 };

/* Hand-defined 5x7 uppercase glyphs; all game UI is embedded, not loaded. */
typedef struct { char ch; unsigned char row[7]; } Glyph;
static const Glyph glyphs[] = {
    {' ',{0,0,0,0,0,0,0}},
    {'0',{14,17,19,21,25,17,14}},
    {'1',{4,12,4,4,4,4,14}},
    {'2',{14,17,1,2,4,8,31}},
    {'3',{30,1,1,14,1,1,30}},
    {'4',{2,6,10,18,31,2,2}},
    {'5',{31,16,16,30,1,1,30}},
    {'6',{14,16,16,30,17,17,14}},
    {'7',{31,1,2,4,8,8,8}},
    {'8',{14,17,17,14,17,17,14}},
    {'9',{14,17,17,15,1,1,14}},
    {'A',{14,17,17,31,17,17,17}},
    {'B',{30,17,17,30,17,17,30}},
    {'C',{15,16,16,16,16,16,15}},
    {'D',{30,17,17,17,17,17,30}},
    {'E',{31,16,16,30,16,16,31}},
    {'F',{31,16,16,30,16,16,16}},
    {'G',{15,16,16,23,17,17,15}},
    {'H',{17,17,17,31,17,17,17}},
    {'I',{14,4,4,4,4,4,14}},
    {'J',{7,2,2,2,18,18,12}},
    {'K',{17,18,20,24,20,18,17}},
    {'L',{16,16,16,16,16,16,31}},
    {'M',{17,27,21,21,17,17,17}},
    {'N',{17,25,21,19,17,17,17}},
    {'O',{14,17,17,17,17,17,14}},
    {'P',{30,17,17,30,16,16,16}},
    {'Q',{14,17,17,17,21,18,13}},
    {'R',{30,17,17,30,20,18,17}},
    {'S',{15,16,16,14,1,1,30}},
    {'T',{31,4,4,4,4,4,4}},
    {'U',{17,17,17,17,17,17,14}},
    {'V',{17,17,17,17,17,10,4}},
    {'W',{17,17,17,21,21,21,10}},
    {'X',{17,17,10,4,10,17,17}},
    {'Y',{17,17,10,4,4,4,4}},
    {'Z',{31,1,2,4,8,16,31}},
    {':',{0,4,4,0,4,4,0}},
    {'-',{0,0,0,31,0,0,0}},
    {'>',{16,8,4,2,4,8,16}},
    {'+',{4,4,4,31,4,4,4}},
    {'=',{0,31,0,31,0,0,0}},
    {'|',{4,4,4,4,4,4,4}},
    {'/',{1,2,2,4,8,8,16}}
};

void render_init(void)
{
    int i;
    for (i = 0; i <= CIRCLE_SEGMENTS; ++i) {
        float angle = 6.28318530718f * ((float)i / CIRCLE_SEGMENTS);
        circle_cos[i] = cosf(angle);
        circle_sin[i] = sinf(angle);
    }
}

static void emit_vertex(float x, float y, GXColor color)
{
    GX_Position3f32(x, y, -5.0f);
    GX_Color4u8(color.r, color.g, color.b, color.a);
}

static void emit_quad(float x, float y, float w, float h, GXColor color)
{
    emit_vertex(x, y, color);
    emit_vertex(x + w, y, color);
    emit_vertex(x + w, y + h, color);
    emit_vertex(x, y + h, color);
}

static void draw_rectangle(float x, float y, float w, float h, GXColor color)
{
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    emit_quad(x, y, w, h, color);
    GX_End();
}

static void draw_disc(float x, float y, float radius, GXColor color)
{
    int i;
    GX_Begin(GX_TRIANGLEFAN, GX_VTXFMT0, CIRCLE_SEGMENTS + 2);
    emit_vertex(x, y, color);
    for (i = 0; i <= CIRCLE_SEGMENTS; ++i)
        emit_vertex(x + circle_cos[i] * radius,
                    y + circle_sin[i] * radius, color);
    GX_End();
}

static const Glyph *get_glyph(char ch)
{
    unsigned i;
    for (i = 0; i < sizeof(glyphs) / sizeof(glyphs[0]); ++i)
        if (glyphs[i].ch == ch) return &glyphs[i];
    return &glyphs[0];
}

static void draw_text(const char *text, float x, float y, float scale)
{
    int pixels = 0;
    int row, col;
    const char *it;
    float current_x = x;
    for (it = text; *it; ++it) {
        const Glyph *glyph = get_glyph(*it);
        for (row = 0; row < 7; ++row)
            for (col = 0; col < 5; ++col)
                if (glyph->row[row] & (1u << (4 - col))) ++pixels;
    }
    if (!pixels) return;
    GX_Begin(GX_QUADS, GX_VTXFMT0, pixels * 4);
    for (it = text; *it; ++it) {
        const Glyph *glyph = get_glyph(*it);
        for (row = 0; row < 7; ++row)
            for (col = 0; col < 5; ++col)
                if (glyph->row[row] & (1u << (4 - col)))
                    emit_quad(current_x + col * scale, y + row * scale,
                              scale, scale, ASTEROIDS_WHITE);
        current_x += 6.0f * scale;
    }
    GX_End();
}

static void draw_ship_at(float x, float y, float angle)
{
    const float left = angle - 2.5f;
    const float right = angle + 2.5f;
    GX_Begin(GX_TRIANGLES, GX_VTXFMT0, 3);
    emit_vertex(x + sinf(angle) * SHIP_RADIUS,
                y - cosf(angle) * SHIP_RADIUS, ASTEROIDS_WHITE);
    emit_vertex(x + sinf(left) * SHIP_RADIUS,
                y - cosf(left) * SHIP_RADIUS, ASTEROIDS_WHITE);
    emit_vertex(x + sinf(right) * SHIP_RADIUS,
                y - cosf(right) * SHIP_RADIUS, ASTEROIDS_WHITE);
    GX_End();
}

static void draw_asteroid_at(float x, float y, unsigned char size)
{
    float radius = game_asteroid_radius(size);
    draw_disc(x, y, radius, COLOR_BODY);
    /* Hardcoded offsets: ALL asteroids of a size share these exact craters. */
    if (size == ASTEROID_LARGE) {
        draw_disc(x - radius * 0.34f, y - radius * 0.30f, radius * 0.18f, COLOR_CRATER);
        draw_disc(x + radius * 0.36f, y - radius * 0.12f, radius * 0.21f, COLOR_CRATER);
        draw_disc(x + radius * 0.02f, y + radius * 0.41f, radius * 0.17f, COLOR_CRATER);
    } else if (size == ASTEROID_MEDIUM) {
        draw_disc(x - radius * 0.28f, y - radius * 0.23f, radius * 0.22f, COLOR_CRATER);
        draw_disc(x + radius * 0.28f, y + radius * 0.26f, radius * 0.20f, COLOR_CRATER);
    } else {
        draw_disc(x + radius * 0.08f, y + radius * 0.05f, radius * 0.30f, COLOR_CRATER);
    }
}

/* Copy only when a shape overlaps a screen edge, not 9 copies every frame. */
static void draw_wrapped_asteroid(const GameAsteroid *asteroid)
{
    float xs[3], ys[3];
    float radius = game_asteroid_radius(asteroid->size);
    int xn = 1, yn = 1, i, j;
    xs[0] = asteroid->position.x;
    ys[0] = asteroid->position.y;
    if (xs[0] < radius) xs[xn++] = xs[0] + GAME_WORLD_W;
    else if (xs[0] > GAME_WORLD_W - radius) xs[xn++] = xs[0] - GAME_WORLD_W;
    if (ys[0] < radius) ys[yn++] = ys[0] + GAME_WORLD_H;
    else if (ys[0] > GAME_WORLD_H - radius) ys[yn++] = ys[0] - GAME_WORLD_H;
    for (i = 0; i < xn; ++i)
        for (j = 0; j < yn; ++j)
            draw_asteroid_at(xs[i], ys[j], asteroid->size);
}

static void draw_wrapped_ship(const Game *game)
{
    const float radius = SHIP_RADIUS;
    float xs[3], ys[3];
    int xn = 1, yn = 1, i, j;
    xs[0] = game->ship_position.x;
    ys[0] = game->ship_position.y;
    if (xs[0] < radius) xs[xn++] = xs[0] + GAME_WORLD_W;
    else if (xs[0] > GAME_WORLD_W - radius) xs[xn++] = xs[0] - GAME_WORLD_W;
    if (ys[0] < radius) ys[yn++] = ys[0] + GAME_WORLD_H;
    else if (ys[0] > GAME_WORLD_H - radius) ys[yn++] = ys[0] - GAME_WORLD_H;
    for (i = 0; i < xn; ++i)
        for (j = 0; j < yn; ++j)
            draw_ship_at(xs[i], ys[j], game->ship_angle);
}

void render_game(const Game *game)
{
    int i;
    char line[64];
    for (i = 0; i < GAME_MAX_ASTEROIDS; ++i)
        if (game->asteroids[i].active) draw_wrapped_asteroid(&game->asteroids[i]);

    for (i = 0; i < GAME_MAX_BULLETS; ++i)
        if (game->bullets[i].active) {
            float x = game->bullets[i].position.x;
            float y = game->bullets[i].position.y;
            draw_rectangle(x - 2.0f, y - 2.0f, 4.0f, 4.0f, ASTEROIDS_WHITE);
            if (x < 2.0f) draw_rectangle(x + GAME_WORLD_W - 2.0f, y - 2.0f, 4, 4, ASTEROIDS_WHITE);
            if (x > GAME_WORLD_W - 2.0f) draw_rectangle(x - GAME_WORLD_W - 2.0f, y - 2.0f, 4, 4, ASTEROIDS_WHITE);
            if (y < 2.0f) draw_rectangle(x - 2.0f, y + GAME_WORLD_H - 2.0f, 4, 4, ASTEROIDS_WHITE);
            if (y > GAME_WORLD_H - 2.0f) draw_rectangle(x - 2.0f, y - GAME_WORLD_H - 2.0f, 4, 4, ASTEROIDS_WHITE);
        }

    if (game->ship_alive &&
        (game->invulnerable_time <= 0.0f || ((int)(game->invulnerable_time * 8.0f) & 1))) {
        draw_wrapped_ship(game);
    }

    snprintf(line, sizeof(line), "SCORE %06d", game->score);
    draw_text(line, 22.0f, 28.0f, 2.0f);
    snprintf(line, sizeof(line), "LIVES %d  WAVE %d", game->lives, game->wave);
    draw_text(line, 22.0f, 47.0f, 2.0f);

    if (game->game_over) {
        draw_text("GAME OVER", 211.0f, 210.0f, 4.0f);
        draw_text("PRESS X TO RESTART", 194.0f, 259.0f, 2.0f);
    }
    draw_text("LEFT+RIGHT = TURN | B = THRUST | A = FIRE | START = EXIT", 24.0f, 449.0f, 1.5f);

    /* Draw a thin permanent border just inside the true world boundaries.
       Two logical units help keep it visible after video scaling/filtering. */
    draw_rectangle(0.0f, 0.0f, GAME_WORLD_W, 2.0f, ASTEROIDS_WHITE);
    draw_rectangle(0.0f, GAME_WORLD_H - 2.0f, GAME_WORLD_W, 2.0f, ASTEROIDS_WHITE);
    draw_rectangle(0.0f, 2.0f, 2.0f, GAME_WORLD_H - 4.0f, ASTEROIDS_WHITE);
    draw_rectangle(GAME_WORLD_W - 2.0f, 2.0f, 2.0f, GAME_WORLD_H - 4.0f, ASTEROIDS_WHITE);
}
