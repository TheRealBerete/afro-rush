#include <vita2d.h>
#include <psp2/ctrl.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/sysmodule.h>
#include "game.h"
#include "audio.h"

static vita2d_texture *load(const char *path) {
    return vita2d_load_PNG_file(path);
}

int main(int argc, char *argv[]) {
    vita2d_init();
    vita2d_set_clear_color(RGBA8(0x10, 0x10, 0x18, 0xFF));

    sceSysmoduleLoadModule(SCE_SYSMODULE_PGF);
    vita2d_pgf *font = vita2d_load_default_pgf();

    audio_start_bgm("app0:/assets/sound/sound.wav");

    Assets assets;
    assets.player = load("app0:/assets/player_run_1.png");
    assets.obstacle[0] = load("app0:/assets/obstacle_1.png");
    assets.obstacle[1] = load("app0:/assets/obstacle_2.png");
    assets.obstacle[2] = load("app0:/assets/obstacle_3.png");
    assets.collectible = load("app0:/assets/collectible.png");
    assets.parallax[0] = load("app0:/assets/couche-1-nuage.png");
    assets.parallax[1] = load("app0:/assets/couche-2-paysage.png");
    assets.parallax[2] = load("app0:/assets/couche-3-terre.png");

    Game game;
    game_init(&game);

    SceCtrlData pad;
    unsigned int prev_buttons = 0;
    const float dt = 1.0f / 60.0f;

    while (1) {
        sceCtrlPeekBufferPositive(0, &pad, 1);

        if (pad.buttons & SCE_CTRL_START) {
            break;
        }

        /* saut/valider sur l'appui (pas en continu si le bouton reste enfonce) */
        int x_pressed = (pad.buttons & SCE_CTRL_CROSS) && !(prev_buttons & SCE_CTRL_CROSS);
        prev_buttons = pad.buttons;

        game_update(&game, x_pressed, dt);

        vita2d_start_drawing();
        vita2d_clear_screen();
        game_render(&game, &assets, font);
        vita2d_end_drawing();
        vita2d_swap_buffers();
    }

    audio_stop_bgm();
    vita2d_free_pgf(font);
    vita2d_fini();
    sceKernelExitProcess(0);
    return 0;
}
