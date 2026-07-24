#include <math.h>
#include <stdlib.h>
#include <psp2/kernel/processmgr.h>
#include "game.h"

#define GRAVITY        1400.0f
#define JUMP_VELOCITY  -560.0f
/* hauteur max atteignable par un saut: v^2/(2*g) ~= 112px. Tous les obstacles
   doivent rester nettement en-dessous pour rester franchissables. */

#define PLAYER_W 72
#define PLAYER_H 110
#define PLAYER_X 120

#define BASE_SPEED     320.0f
#define MAX_SPEED      760.0f
#define SPEED_RAMP     2.2f   /* px/s gagnes par metre parcouru */

#define MIN_OBSTACLE_GAP 260.0f
#define MAX_OBSTACLE_GAP 460.0f

#define COLLECTIBLE_SIZE 40
#define MIN_COLLECTIBLE_GAP 300.0f
#define MAX_COLLECTIBLE_GAP 600.0f

/* dimensions a l'ecran par type d'obstacle, calees sur les proportions
   reelles des sprites et plafonnees pour rester sautables (<112px de haut) */
static const int OBSTACLE_SIZES[NUM_OBSTACLE_TYPES][2] = {
    { 88, 65 },  /* obstacle_1: large et bas */
    { 65, 95 },  /* obstacle_2: poteau haut */
    { 76, 70 },  /* obstacle_3 */
};

/* vitesse de defilement relative de chaque couche de parallax (0=fond, 2=premier plan) */
static const float PARALLAX_FACTOR[NUM_PARALLAX_LAYERS] = { 0.2f, 0.5f, 0.8f };

static float rand_range(float min, float max) {
    return min + ((float)rand() / (float)RAND_MAX) * (max - min);
}

static void reset_run(Game *g) {
    g->player.x = PLAYER_X;
    g->player.y = GROUND_Y - PLAYER_H;
    g->player.vy = 0;
    g->player.w = PLAYER_W;
    g->player.h = PLAYER_H;
    g->player.on_ground = 1;

    for (int i = 0; i < MAX_OBSTACLES; i++) g->obstacles[i].active = 0;
    for (int i = 0; i < MAX_COLLECTIBLES; i++) g->collectibles[i].active = 0;

    g->scroll_speed = BASE_SPEED;
    g->distance_since_last_obstacle = 0;
    g->distance_since_last_collectible = MIN_COLLECTIBLE_GAP * 0.5f;
    g->distance_total = 0;
    g->score = 0;
    g->state = STATE_PLAYING;
}

void game_init(Game *g) {
    srand((unsigned int)sceKernelGetProcessTimeWide());
    g->best_score = 0;
    for (int i = 0; i < NUM_PARALLAX_LAYERS; i++) g->parallax_offset[i] = 0;
    reset_run(g);
}

static int aabb_overlap(float ax, float ay, int aw, int ah,
                         float bx, float by, int bw, int bh) {
    return ax < bx + bw && ax + aw > bx &&
           ay < by + bh && ay + ah > by;
}

static void spawn_obstacle(Game *g) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!g->obstacles[i].active) {
            int type = rand() % NUM_OBSTACLE_TYPES;
            g->obstacles[i].active = 1;
            g->obstacles[i].type = type;
            g->obstacles[i].w = OBSTACLE_SIZES[type][0];
            g->obstacles[i].h = OBSTACLE_SIZES[type][1];
            g->obstacles[i].x = SCREEN_W + 10;
            g->obstacles[i].y = GROUND_Y - g->obstacles[i].h;
            return;
        }
    }
}

static void spawn_collectible(Game *g) {
    for (int i = 0; i < MAX_COLLECTIBLES; i++) {
        if (!g->collectibles[i].active) {
            g->collectibles[i].active = 1;
            g->collectibles[i].x = SCREEN_W + 10;
            /* flotte a hauteur variable au-dessus du sol, parfois a hauteur de saut */
            g->collectibles[i].y = GROUND_Y - COLLECTIBLE_SIZE - rand_range(0, 140);
            g->collectibles[i].w = COLLECTIBLE_SIZE;
            g->collectibles[i].h = COLLECTIBLE_SIZE;
            return;
        }
    }
}

void game_update(Game *g, int jump_pressed, float dt) {
    if (g->state == STATE_GAME_OVER) {
        if (jump_pressed) {
            if (g->score > g->best_score) g->best_score = g->score;
            reset_run(g);
        }
        return;
    }

    /* difficulte croissante avec la distance parcourue */
    g->scroll_speed = BASE_SPEED + g->distance_total * SPEED_RAMP;
    if (g->scroll_speed > MAX_SPEED) g->scroll_speed = MAX_SPEED;

    float move = g->scroll_speed * dt;
    g->distance_total += move;
    g->distance_since_last_obstacle += move;
    g->distance_since_last_collectible += move;
    g->score = (int)(g->distance_total / 10.0f);

    for (int i = 0; i < NUM_PARALLAX_LAYERS; i++) {
        g->parallax_offset[i] += move * PARALLAX_FACTOR[i];
    }

    /* physique du joueur */
    if (jump_pressed && g->player.on_ground) {
        g->player.vy = JUMP_VELOCITY;
        g->player.on_ground = 0;
    }
    g->player.vy += GRAVITY * dt;
    g->player.y += g->player.vy * dt;
    if (g->player.y + g->player.h >= GROUND_Y) {
        g->player.y = GROUND_Y - g->player.h;
        g->player.vy = 0;
        g->player.on_ground = 1;
    }

    /* spawn obstacles selon la distance, pas le temps: garde un ecart jouable a toute vitesse */
    if (g->distance_since_last_obstacle > rand_range(MIN_OBSTACLE_GAP, MAX_OBSTACLE_GAP)) {
        spawn_obstacle(g);
        g->distance_since_last_obstacle = 0;
    }
    if (g->distance_since_last_collectible > rand_range(MIN_COLLECTIBLE_GAP, MAX_COLLECTIBLE_GAP)) {
        spawn_collectible(g);
        g->distance_since_last_collectible = 0;
    }

    /* deplacement + collision obstacles */
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!g->obstacles[i].active) continue;
        g->obstacles[i].x -= move;
        if (g->obstacles[i].x + g->obstacles[i].w < 0) {
            g->obstacles[i].active = 0;
            continue;
        }
        if (aabb_overlap(g->player.x, g->player.y, g->player.w, g->player.h,
                          g->obstacles[i].x, g->obstacles[i].y, g->obstacles[i].w, g->obstacles[i].h)) {
            g->state = STATE_GAME_OVER;
        }
    }

    /* deplacement + collision collectibles */
    for (int i = 0; i < MAX_COLLECTIBLES; i++) {
        if (!g->collectibles[i].active) continue;
        g->collectibles[i].x -= move;
        if (g->collectibles[i].x + g->collectibles[i].w < 0) {
            g->collectibles[i].active = 0;
            continue;
        }
        if (aabb_overlap(g->player.x, g->player.y, g->player.w, g->player.h,
                          g->collectibles[i].x, g->collectibles[i].y, g->collectibles[i].w, g->collectibles[i].h)) {
            g->collectibles[i].active = 0;
            g->score += 50;
        }
    }
}

/* dessine une couche de parallax qui defile en boucle horizontalement,
   alignee par le bas sur la ligne de sol */
static void render_parallax_layer(vita2d_texture *tex, float offset) {
    if (!tex) return;
    int tex_w = vita2d_texture_get_width(tex);
    int tex_h = vita2d_texture_get_height(tex);
    float scale = (float)SCREEN_W / (float)tex_w;
    float scaled_w = tex_w * scale;
    float scaled_h = tex_h * scale;
    float draw_y = GROUND_Y - scaled_h;

    float x = -fmodf(offset, scaled_w);
    vita2d_draw_texture_scale(tex, x, draw_y, scale, scale);
    vita2d_draw_texture_scale(tex, x + scaled_w, draw_y, scale, scale);
}

void game_render(Game *g, const Assets *assets, vita2d_pgf *font) {
    for (int i = 0; i < NUM_PARALLAX_LAYERS; i++) {
        render_parallax_layer(assets->parallax[i], g->parallax_offset[i]);
    }

    if (assets->player) {
        float scale_x = (float)g->player.w / vita2d_texture_get_width(assets->player);
        float scale_y = (float)g->player.h / vita2d_texture_get_height(assets->player);
        vita2d_draw_texture_scale(assets->player, g->player.x, g->player.y, scale_x, scale_y);
    }

    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!g->obstacles[i].active) continue;
        vita2d_texture *tex = assets->obstacle[g->obstacles[i].type];
        if (!tex) continue;
        float scale_x = (float)g->obstacles[i].w / vita2d_texture_get_width(tex);
        float scale_y = (float)g->obstacles[i].h / vita2d_texture_get_height(tex);
        vita2d_draw_texture_scale(tex, g->obstacles[i].x, g->obstacles[i].y, scale_x, scale_y);
    }

    for (int i = 0; i < MAX_COLLECTIBLES; i++) {
        if (!g->collectibles[i].active) continue;
        if (!assets->collectible) continue;
        float scale_x = (float)g->collectibles[i].w / vita2d_texture_get_width(assets->collectible);
        float scale_y = (float)g->collectibles[i].h / vita2d_texture_get_height(assets->collectible);
        vita2d_draw_texture_scale(assets->collectible, g->collectibles[i].x, g->collectibles[i].y, scale_x, scale_y);
    }

    vita2d_pgf_draw_textf(font, 20, 40, RGBA8(0xFF, 0xFF, 0xFF, 0xFF), 1.2f, "AFRO RUSH   score %d", g->score);

    if (g->state == STATE_GAME_OVER) {
        vita2d_pgf_draw_textf(font, SCREEN_W / 2 - 160, SCREEN_H / 2 - 20, RGBA8(0xFF, 0x40, 0x40, 0xFF), 1.4f, "GAME OVER");
        vita2d_pgf_draw_textf(font, SCREEN_W / 2 - 220, SCREEN_H / 2 + 30, RGBA8(0xFF, 0xFF, 0xFF, 0xFF), 1.0f, "Meilleur score: %d", g->best_score);
        vita2d_pgf_draw_textf(font, SCREEN_W / 2 - 220, SCREEN_H / 2 + 60, RGBA8(0xCC, 0xCC, 0xCC, 0xFF), 1.0f, "Appuie sur X pour recommencer");
    }
}
