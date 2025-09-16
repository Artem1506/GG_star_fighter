#ifndef GAME_H
#define GAME_H

#include <Arduino.h>
#include "entities.h"

// ==================== ��������� ���� ====================
enum GameState {
    STATE_INTRO,
    STATE_MENU,
    STATE_PLAYING,
    STATE_GAMEOVER
};

// ==================== ��������� ====================
void gameInit();
void gameUpdate();
void gameRender();

#endif
