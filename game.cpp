#include "game.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "entities.h"
#include <SD_MMC.h>

// ==================== Константы ====================
#define MAX_BULLETS   10
#define MAX_ASTEROIDS 20
#define BULLET_DELAY  300   // мс
#define SHIP_SPEED    2.0f
#define BULLET_SPEED  4.0f
#define ASTEROID_BASE_SPEED 1.0f

// ==================== Глобальные переменные ====================
static GameState state = STATE_INTRO;
static Ship player;
static Bullet bullets[MAX_BULLETS];
static Asteroid asteroids[MAX_ASTEROIDS];

static unsigned long lastBulletTime = 0;
static unsigned long score = 0;
static unsigned long highscore = 0;

// ==================== Вспомогательные ====================
static void resetGame();
static void spawnAsteroid(bool forceComet = false);
static void saveHighscore();
static void loadHighscore();

// ==================== Инициализация ====================
void gameInit() {
    loadHighscore();

    // Начинаем с логотипа
    drawLogo();
    playRandomIntro();
    delay(3000);

    state = STATE_MENU;
}
extern SpawnManager spawnManager;

void GameManager::updatePlayState() {
    // Обновление игровой логики
    ship.update();

    // Обновление системы спавна с учетом счета
    spawnManager.update(score);

    // Обновление всех астероидов
    for (auto& asteroid : spawnManager.asteroids) {
        asteroid->update();
    }
}

void GameManager::changeState(GameState newState) {
    currentState = newState;
    stateStartTime = millis();

    switch (newState) {
    case STATE_LOGO:
        // Инициализация логотипа
        break;
    case STATE_MENU:
        // Запуск меню музыки
        audio.playRandomIntro();
        break;
    case STATE_PLAY:
        // Сброс игровых параметров
        resetGame();
        audio.playRandomMain();
        break;
    case STATE_GAME_OVER:
        // Проверка рекорда
        checkHighScore();
        audio.playRandomGameOver();
        break;
    }
}

// ==================== Основной цикл ====================
void gameUpdate() {
    updateInput();
    InputState in = getInput();

    switch (state) {
    case STATE_MENU:
        drawMenuAnimation();
        if (in.encoderPressed) {
            resetGame();
            state = STATE_PLAYING;
            playRandomMain();
        }
        break;

    case STATE_PLAYING:
        // Управление кораблем
        player.angle = in.encoderAngle;
        if (in.buttonA) {
            player.x += SHIP_SPEED * cos(radians(player.angle));
            player.y += SHIP_SPEED * sin(radians(player.angle));
        }

        // Телепортация на границах
        if (player.x < 0) player.x = 127;
        if (player.x > 127) player.x = 0;
        if (player.y < 0) player.y = 63;
        if (player.y > 63) player.y = 0;

        // Стрельба
        if (in.buttonB && millis() - lastBulletTime > BULLET_DELAY) {
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!bullets[i].active) {
                    bullets[i].x = player.x;
                    bullets[i].y = player.y;
                    bullets[i].dx = BULLET_SPEED * cos(radians(player.angle));
                    bullets[i].dy = BULLET_SPEED * sin(radians(player.angle));
                    bullets[i].active = true;
                    bullets[i].spawnTime = millis();
                    playEffect(SOUND_LASER);
                    lastBulletTime = millis();
                    break;
                }
            }
        }

        // Обновление пуль
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) {
                bullets[i].x += bullets[i].dx;
                bullets[i].y += bullets[i].dy;
                if (millis() - bullets[i].spawnTime > 2000) {
                    bullets[i].active = false;
                }
            }
        }

        // Обновление астероидов
        for (int i = 0; i < MAX_ASTEROIDS; i++) {
            if (asteroids[i].active) {
                asteroids[i].x += asteroids[i].dx;
                asteroids[i].y += asteroids[i].dy;

                // Телепортация
                if (asteroids[i].x < 0) asteroids[i].x = 127;
                if (asteroids[i].x > 127) asteroids[i].x = 0;
                if (asteroids[i].y < 0) asteroids[i].y = 63;
                if (asteroids[i].y > 63) asteroids[i].y = 0;

                // Проверка столкновения с пулей
                for (int b = 0; b < MAX_BULLETS; b++) {
                    if (bullets[b].active &&
                        abs(bullets[b].x - asteroids[i].x) < 3 &&
                        abs(bullets[b].y - asteroids[i].y) < 3) {
                        bullets[b].active = false;
                        asteroids[i].active = false;
                        score++;
                        playEffect(SOUND_HIT);
                        spawnAsteroid();
                    }
                }

                // Проверка столкновения с кораблем
                if (abs(player.x - asteroids[i].x) < 4 &&
                    abs(player.y - asteroids[i].y) < 4) {
                    player.alive = false;
                    state = STATE_GAMEOVER;
                    playEffect(SOUND_EXPLOSION);
                    saveHighscore();
                }
            }
        }

        break;

    case STATE_GAMEOVER:
        drawGameOver(score, highscore);
        if (in.encoderPressed) {
            state = STATE_MENU;
        }
        break;
    }
}

// ==================== Рендер ====================
void gameRender() {
    switch (state) {
    case STATE_MENU:
        drawMenuAnimation();
        break;
    case STATE_PLAYING:
        clearScreen();
        drawShip(player.x, player.y, player.angle, false);
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) drawBullet(bullets[i].x, bullets[i].y);
        }
        // астероиды
        for (int i = 0; i < MAX_ASTEROIDS; i++) {
            if (asteroids[i].active) {
                drawAsteroid((int)asteroids[i].x, (int)asteroids[i].y, asteroids[i].comet);
            }
        }
        drawScore(score);
        break;
    case STATE_GAMEOVER:
        drawGameOver(score, highscore);
        break;
    default:
        break;
    }
}

// ==================== Вспомогательные ====================
static void resetGame() {
    player.x = 64;
    player.y = 32;
    player.angle = 0;
    player.alive = true;

    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
    for (int i = 0; i < MAX_ASTEROIDS; i++) asteroids[i].active = false;

    score = 0;
    spawnAsteroid();
}

static void spawnAsteroid(bool forceComet) {
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!asteroids[i].active) {
            asteroids[i].x = random(128);
            asteroids[i].y = random(64);
            asteroids[i].comet;
            float angle = random(360) * PI / 180.0f;
            float speed = ASTEROID_BASE_SPEED;

            // Каждые 5 очков появляется больше астероидов
            if (score > 0 && score % 5 == 0) {
                speed *= 1.2;
            }

            bool comet = (random(100) < (score * 5)) || forceComet;
            if (comet) speed *= 1.5;

            asteroids[i].dx = cos(angle) * speed;
            asteroids[i].dy = sin(angle) * speed;
            asteroids[i].active = true;
            asteroids[i].comet = comet;
            break;
        }
    }
}

static void saveHighscore() {
    if (score > highscore) {
        highscore = score;
        File f = SD_MMC.open("/highscore.txt", FILE_WRITE);
        if (f) {
            f.println(highscore);
            f.close();
        }
    }
}

static void loadHighscore() {
    File f = SD_MMC.open("/highscore.txt");
    if (f) {
        highscore = f.parseInt();
        f.close();
    }
}
