#ifndef GAME_H
#define GAME_H

#include <Arduino.h>
#include "entities.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "storage.h"

// ==================== ÑÎÑÒÎßÍÈß ÈÃĞÛ ====================
enum GameState {
    STATE_LOGO,
    STATE_MENU,
    STATE_PLAY,
    STATE_GAME_OVER
};

// ==================== ÎÑÍÎÂÍÎÉ ÊËÀÑÑ ÓÏĞÀÂËÅÍÈß ====================
class GameManager {
public:
    void init();
    void update();
    void render();
    void changeState(GameState newState);

    GameState getCurrentState() const { return currentState; }
    int getScore() const { return score; }
    int getHighScore() const { return highScore; }

private:
    void resetGame();
    void spawnAsteroid(bool forceComet = false);
    void updateLogo();
    void updateMenu(const InputState& input);
    void updatePlay(const InputState& input);
    void updateGameOver(const InputState& input);
    void checkCollisions();
    void checkHighScore();

    // Èãğîâûå îáúåêòû
    Ship playerShip;
    Bullet bullets[MAX_BULLETS];
    Asteroid asteroids[MAX_ASTEROIDS];

    // Ñîñòîÿíèå èãğû
    GameState currentState = STATE_LOGO;
    int score = 0;
    int highScore = 0;
    uint32_t stateStartTime = 0;
    uint32_t lastBulletTime = 0;
    uint32_t lastAsteroidSpawn = 0;
    uint8_t activeBullets = 0;
    uint8_t activeAsteroids = 0;
};

#endif
