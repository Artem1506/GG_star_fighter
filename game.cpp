#include "game.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "storage.h"
#include "entities.h"
#include <SD_MMC.h>

// ==================== ВНЕШНИЕ ОБЪЕКТЫ ====================
extern Graphics graphics;   
extern InputManager input;  
extern AudioManager audio;   

// ==================== КОНСТАНТЫ ====================
constexpr uint32_t LOGO_DISPLAY_TIME = 2000;

// ==================== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ====================
GameManager gameManager;
StorageManager storageManager;

// ==================== ИНИЦИАЛИЗАЦИЯ ====================
void GameManager::init() {
    // Загрузка рекорда
    highScore = storageManager.readHighScore();

    // Начальное состояние
    changeState(STATE_LOGO);
}

void GameManager::changeState(GameState newState) {
    currentState = newState;
    stateStartTime = millis();

    switch (newState) {
    case STATE_LOGO:
        // Отрисовка логотипа на 2 секунды
        break;

    case STATE_MENU:
        audio.playRandomIntro();
        break;

    case STATE_PLAY:
        resetGame();
        audio.playRandomMain();
        break;

    case STATE_GAME_OVER:
        checkHighScore();
        audio.playRandomGameOver();
        break;
    }
}

// ==================== ОСНОВНОЙ ЦИКЛ ====================
void GameManager::update() {
    InputState inputState = input.getState();

    switch (currentState) {
    case STATE_LOGO:
        updateLogo();
        break;

    case STATE_MENU:
        updateMenu(inputState);
        break;

    case STATE_PLAY:
        updatePlay(inputState);
        break;

    case STATE_GAME_OVER:
        updateGameOver(inputState);
        break;
    }
}

void GameManager::render() {
    graphics.clear();

    switch (currentState) {
    case STATE_LOGO:
        graphics.drawLogo();
        break;

    case STATE_MENU:
        graphics.drawStartMenu((millis() - stateStartTime) / 50); // frame counter
        break;

    case STATE_PLAY:
        graphics.drawGameBackground();

        // Отрисовка корабля
        graphics.drawShip(playerShip.base.x, playerShip.base.y,
            playerShip.rotation, playerShip.boosting);

        // Отрисовка пуль
        for (uint8_t i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].base.active) {
                float angle = atan2(bullets[i].base.vy, bullets[i].base.vx) * 180 / M_PI;
                graphics.drawBullet(bullets[i].base.x, bullets[i].base.y, angle);
            }
        }

        // Отрисовка астероидов
        for (uint8_t i = 0; i < MAX_ASTEROIDS; i++) {
            if (asteroids[i].base.active) {
                if (asteroids[i].isComet) {
                    float direction = atan2(asteroids[i].base.vy, asteroids[i].base.vx) * 180 / M_PI;
                    graphics.drawComet(asteroids[i].base.x, asteroids[i].base.y, direction);
                }
                else {
                    graphics.drawAsteroid(asteroids[i].base.x, asteroids[i].base.y, asteroids[i].size);
                }
            }
        }

        graphics.drawScore(score);
        break;

    case STATE_GAME_OVER:
        graphics.drawGameOver(score, highScore);
        break;
    }
}

// ==================== СОСТОЯНИЕ ЛОГОТИПА ====================
void GameManager::updateLogo() {
    if (millis() - stateStartTime >= LOGO_DISPLAY_TIME) {
        changeState(STATE_MENU);
    }
}

// ==================== СОСТОЯНИЕ МЕНЮ ====================
void GameManager::updateMenu(const InputState& inputState) {
    if (inputState.encoderPressed) {
        changeState(STATE_PLAY);
    }
}

// ==================== СОСТОЯНИЕ ИГРЫ ====================
void GameManager::updatePlay(const InputState& inputState) {
    // Обновление корабля
    playerShip.rotation = inputState.encoderAngle;

    if (inputState.buttonA) {
        // Движение корабля
        float rad = playerShip.rotation * M_PI / 180.0f;
        playerShip.base.vx += cos(rad) * SHIP_SPEED;
        playerShip.base.vy += sin(rad) * SHIP_SPEED;
        playerShip.boosting = true;
    }
    else {
        playerShip.boosting = false;
    }

    // Обновление позиции корабля
    playerShip.base.x += playerShip.base.vx;
    playerShip.base.y += playerShip.base.vy;

    // Телепортация на границах
    if (playerShip.base.x < 0) playerShip.base.x = SCREEN_WIDTH;
    if (playerShip.base.x > SCREEN_WIDTH) playerShip.base.x = 0;
    if (playerShip.base.y < 0) playerShip.base.y = SCREEN_HEIGHT;
    if (playerShip.base.y > SCREEN_HEIGHT) playerShip.base.y = 0;

    // Стрельба
    if (inputState.buttonB && millis() - lastBulletTime > BULLET_DELAY && activeBullets < MAX_BULLETS) {
        for (uint8_t i = 0; i < MAX_BULLETS; i++) {
            if (!bullets[i].base.active) {
                float rad = playerShip.rotation * M_PI / 180.0f;
                bullets[i].base.x = playerShip.base.x;
                bullets[i].base.y = playerShip.base.y;
                bullets[i].base.vx = cos(rad) * BULLET_SPEED;
                bullets[i].base.vy = sin(rad) * BULLET_SPEED;
                bullets[i].base.active = true;
                bullets[i].spawnTime = millis();
                activeBullets++;
                lastBulletTime = millis();
                audio.playLaser();
                break;
            }
        }
    }

    // Обновление пуль
    for (uint8_t i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].base.active) {
            bullets[i].base.x += bullets[i].base.vx;
            bullets[i].base.y += bullets[i].base.vy;

            // Удаление старых пуль
            if (millis() - bullets[i].spawnTime > 2000) {
                bullets[i].base.active = false;
                activeBullets--;
            }
        }
    }

    // Спавн астероидов
    if (activeAsteroids < 1 + (score / 5) && millis() - lastAsteroidSpawn > 2000) {
        spawnAsteroid();
        lastAsteroidSpawn = millis();
    }

    // Обновление астероидов
    for (uint8_t i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].base.active) {
            asteroids[i].base.x += asteroids[i].base.vx;
            asteroids[i].base.y += asteroids[i].base.vy;

            // Телепортация астероидов
            if (asteroids[i].base.x < -20) asteroids[i].base.x = SCREEN_WIDTH + 20;
            if (asteroids[i].base.x > SCREEN_WIDTH + 20) asteroids[i].base.x = -20;
            if (asteroids[i].base.y < -20) asteroids[i].base.y = SCREEN_HEIGHT + 20;
            if (asteroids[i].base.y > SCREEN_HEIGHT + 20) asteroids[i].base.y = -20;
        }
    }

    // Проверка коллизий
    checkCollisions();
}

// ==================== ПРОВЕРКА КОЛЛИЗИЙ ====================
void GameManager::checkCollisions() {
    // Пули -> Астероиды
    for (uint8_t b = 0; b < MAX_BULLETS; b++) {
        if (!bullets[b].base.active) continue;

        for (uint8_t a = 0; a < MAX_ASTEROIDS; a++) {
            if (!asteroids[a].base.active) continue;

            float dx = bullets[b].base.x - asteroids[a].base.x;
            float dy = bullets[b].base.y - asteroids[a].base.y;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance < 8.0f) { // Примерный радиус столкновения
                bullets[b].base.active = false;
                asteroids[a].base.active = false;
                activeBullets--;
                activeAsteroids--;
                score++;
                audio.playHit();
                break;
            }
        }
    }

    // Корабль -> Астероиды
    for (uint8_t a = 0; a < MAX_ASTEROIDS; a++) {
        if (!asteroids[a].base.active) continue;

        float dx = playerShip.base.x - asteroids[a].base.x;
        float dy = playerShip.base.y - asteroids[a].base.y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < 12.0f) { // Столкновение
            changeState(STATE_GAME_OVER);
            audio.playCrash();
            break;
        }
    }
}

// ==================== СОСТОЯНИЕ GAME OVER ====================
void GameManager::updateGameOver(const InputState& inputState) {
    if (inputState.encoderPressed) {
        changeState(STATE_MENU);
    }
}

// ==================== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ====================
void GameManager::resetGame() {
    // Сброс корабля
    playerShip.base.x = SCREEN_WIDTH / 2;
    playerShip.base.y = SCREEN_HEIGHT / 2;
    playerShip.base.vx = 0;
    playerShip.base.vy = 0;
    playerShip.rotation = 0;
    playerShip.boosting = false;

    // Сброс пуль и астероидов
    for (uint8_t i = 0; i < MAX_BULLETS; i++) {
        bullets[i].base.active = false;
    }
    for (uint8_t i = 0; i < MAX_ASTEROIDS; i++) {
        asteroids[i].base.active = false;
    }

    activeBullets = 0;
    activeAsteroids = 0;
    score = 0;

    // Спавн первого астероида
    spawnAsteroid();
}

void GameManager::spawnAsteroid(bool forceComet) {
    for (uint8_t i = 0; i < MAX_ASTEROIDS; i++) {
        if (!asteroids[i].base.active) {
            // Спавн за пределами экрана
            uint8_t side = random(4);
            switch (side) {
            case 0: asteroids[i].base.x = -20; asteroids[i].base.y = random(SCREEN_HEIGHT); break;
            case 1: asteroids[i].base.x = SCREEN_WIDTH + 20; asteroids[i].base.y = random(SCREEN_HEIGHT); break;
            case 2: asteroids[i].base.x = random(SCREEN_WIDTH); asteroids[i].base.y = -20; break;
            case 3: asteroids[i].base.x = random(SCREEN_WIDTH); asteroids[i].base.y = SCREEN_HEIGHT + 20; break;
            }

            // Случайное направление к центру
            float targetX = SCREEN_WIDTH / 2 + random(-20, 20);
            float targetY = SCREEN_HEIGHT / 2 + random(-20, 20);
            float dx = targetX - asteroids[i].base.x;
            float dy = targetY - asteroids[i].base.y;
            float distance = sqrt(dx * dx + dy * dy);

            asteroids[i].base.vx = (dx / distance) * ASTEROID_BASE_SPEED;
            asteroids[i].base.vy = (dy / distance) * ASTEROID_BASE_SPEED;

            // Шанс спавна кометы
            bool isComet = forceComet || (random(100) < (score * 5));
            if (isComet) {
                asteroids[i].base.vx *= 1.5f;
                asteroids[i].base.vy *= 1.5f;
                asteroids[i].isComet = true;
            }

            asteroids[i].base.active = true;
            activeAsteroids++;
            break;
        }
    }
}

void GameManager::checkHighScore() {
    if (score > highScore) {
        highScore = score;
        storageManager.writeHighScore(highScore);
    }
}
