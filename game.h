#ifndef GAME_H
#define GAME_H

#include <Arduino.h>
#include "entities.h"

// ==================== Состояния игры ====================
enum GameState {
    STATE_INTRO,
    STATE_MENU,
    STATE_PLAYING,
    STATE_GAMEOVER
};

// ==================== Интерфейс ====================
void gameInit();
void gameUpdate();
void gameRender();

#endif
