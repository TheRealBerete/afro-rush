#ifndef GAME_H
#define GAME_H

#include <vita2d.h>

#define SCREEN_W 960
#define SCREEN_H 544
#define GROUND_Y 440

#define MAX_OBSTACLES 6
#define MAX_COLLECTIBLES 4
#define NUM_OBSTACLE_TYPES 3
#define NUM_PARALLAX_LAYERS 3

typedef enum {
    STATE_PLAYING,
    STATE_GAME_OVER
} GameStateEnum;

typedef struct {
    vita2d_texture *player;
    vita2d_texture *obstacle[NUM_OBSTACLE_TYPES];
    vita2d_texture *collectible;
    vita2d_texture *parallax[NUM_PARALLAX_LAYERS]; /* 0=fond lointain (nuages) .. 2=premier plan (terre) */
} Assets;

typedef struct {
    float x, y;
    float vy;
    int w, h;
    int on_ground;
} Player;

typedef struct {
    float x, y;
    int w, h;
    int type;
    int active;
} Obstacle;

typedef struct {
    float x, y;
    int w, h;
    int active;
} Collectible;

typedef struct {
    GameStateEnum state;
    Player player;
    Obstacle obstacles[MAX_OBSTACLES];
    Collectible collectibles[MAX_COLLECTIBLES];
    float scroll_speed;
    float distance_since_last_obstacle;
    float distance_since_last_collectible;
    float distance_total;
    float parallax_offset[NUM_PARALLAX_LAYERS];
    int score;
    int best_score;
} Game;

void game_init(Game *g);
void game_update(Game *g, int jump_pressed, float dt);
void game_render(Game *g, const Assets *assets, vita2d_pgf *font);

#endif
