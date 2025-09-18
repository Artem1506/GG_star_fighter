#include "entities.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include <cmath>

// ==================== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ====================
static Ship playerShip;
static Bullet bullets[MAX_BULLETS];
static Asteroid asteroids[MAX_ASTEROIDS];
static uint8_t activeBullets = 0;
static uint8_t activeAsteroids = 0;
static uint32_t lastBulletTime = 0;

// ==================== ПРЕДВЫЧИСЛЕННЫЕ ТАБЛИЦЫ ====================
constexpr int8_t COS_TABLE[36] = {
    100, 98, 94, 87, 77, 64, 50, 34, 17, 0,
    -17, -34, -50, -64, -77, -87, -94, -98, -100,
    -98, -94, -87, -77, -64, -50, -34, -17, 0,
    17, 34, 50, 64, 77, 87, 94, 98
};

// ==================== ИНИЦИАЛИЗАЦИЯ ====================
void initEntities() {
    resetEntities();
}

void resetEntities() {
    // Сброс корабля
    playerShip.base.x = SCREEN_WIDTH / 2;
    playerShip.base.y = SCREEN_HEIGHT / 2;
    playerShip.base.vx = 0;
    playerShip.base.vy = 0;
    playerShip.rotation = 0;
    playerShip.boosting = false;
    playerShip.lastShot = 0;
    playerShip.base.active = true;

    // Сброс пуль
    for (uint8_t i = 0; i < MAX_BULLETS; i++) {
        bullets[i].base.active = false;
        bullets[i].spawnTime = 0;
    }
    activeBullets = 0;

    // Сброс астероидов
    for (uint8_t i = 0; i < MAX_ASTEROIDS; i++) {
        asteroids[i].base.active = false;
    }
    activeAsteroids = 0;

    lastBulletTime = 0;
}

// ==================== ДОСТУП К ОБЪЕКТАМ ====================
Ship* getPlayerShip() { return &playerShip; }
Bullet* getBullets() { return bullets; }
Asteroid* getAsteroids() { return asteroids; }
uint8_t getActiveBullets() { return activeBullets; }
uint8_t getActiveAsteroids() { return activeAsteroids; }

// ==================== ОБНОВЛЕНИЕ КОРАБЛЯ ====================
void updateShip(Ship* ship, const InputState& input) {
    ship->rotation = input.encoderAngle;

    if (input.buttonA) {
        // Быстрый расчет thrust через таблицу
        int angleIndex = (ship->rotation / 10) % 36;
        float thrustX = COS_TABLE[angleIndex] * 0.01f * SHIP_SPEED;
        float thrustY = COS_TABLE[(angleIndex + 9) % 36] * 0.01f * SHIP_SPEED;

        ship->base.vx += thrustX;
        ship->base.vy += thrustY;
        ship->boosting = true;
    }
    else {
        ship->boosting = false;
    }

    // Обновление позиции
    ship->base.x += ship->base.vx;
    ship->base.y += ship->base.vy;

    // Телепортация на границах
    if (ship->base.x < 0) ship->base.x = SCREEN_WIDTH;
    if (ship->base.x > SCREEN_WIDTH) ship->base.x = 0;
    if (ship->base.y < 0) ship->base.y = SCREEN_HEIGHT;
    if (ship->base.y > SCREEN_HEIGHT) ship->base.y = 0;
}

// ==================== СТРЕЛЬБА ====================
bool fireBullet(Ship* ship) {
    if (millis() - lastBulletTime < BULLET_DELAY) return false;
    if (activeBullets >= MAX_BULLETS) return false;

    for (uint8_t i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].base.active) {
            float rad = ship->rotation * M_PI / 180.0f;
            bullets[i].base.x = ship->base.x;
            bullets[i].base.y = ship->base.y;
            bullets[i].base.vx = cos(rad) * BULLET_SPEED;
            bullets[i].base.vy = sin(rad) * BULLET_SPEED;
            bullets[i].base.active = true;
            bullets[i].spawnTime = millis();

            activeBullets++;
            lastBulletTime = millis();
            return true;
        }
    }
    return false;
}

// ==================== ОБНОВЛЕНИЕ ПУЛЬ ====================
void updateBullets() {
    for (uint8_t i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].base.active) {
            bullets[i].base.x += bullets[i].base.vx;
            bullets[i].base.y += bullets[i].base.vy;

            // Удаление старых пуль
            if (millis() - bullets[i].spawnTime > 2000) {
                bullets[i].base.active = false;
                activeBullets--;
            }

            // Телепортация на границах
            if (bullets[i].base.x < 0) bullets[i].base.x = SCREEN_WIDTH;
            if (bullets[i].base.x > SCREEN_WIDTH) bullets[i].base.x = 0;
            if (bullets[i].base.y < 0) bullets[i].base.y = SCREEN_HEIGHT;
            if (bullets[i].base.y > SCREEN_HEIGHT) bullets[i].base.y = 0;
        }
    }
}

// ==================== ОБНОВЛЕНИЕ АСТЕРОИДОВ ====================
void updateAsteroids(uint16_t score) {
    for (uint8_t i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].base.active) {
            asteroids[i].base.x += asteroids[i].base.vx;
            asteroids[i].base.y += asteroids[i].base.vy;

            // Телепортация на границах
            if (asteroids[i].base.x < -20) asteroids[i].base.x = SCREEN_WIDTH + 20;
            if (asteroids[i].base.x > SCREEN_WIDTH + 20) asteroids[i].base.x = -20;
            if (asteroids[i].base.y < -20) asteroids[i].base.y = SCREEN_HEIGHT + 20;
            if (asteroids[i].base.y > SCREEN_HEIGHT + 20) asteroids[i].base.y = -20;
        }
    }

    // Автоматический спавн при необходимости
    uint8_t targetCount = 1 + (score / 5);
    if (activeAsteroids < targetCount) {
        spawnAsteroid(score);
    }
}

// ==================== СПАВН АСТЕРОИДОВ ====================
void spawnAsteroid(uint16_t score, bool forceComet) {
    if (activeAsteroids >= MAX_ASTEROIDS) return;

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

            // Направление к центру экрана
            float targetX = SCREEN_WIDTH / 2 + random(-20, 20);
            float targetY = SCREEN_HEIGHT / 2 + random(-20, 20);
            float dx = targetX - asteroids[i].base.x;
            float dy = targetY - asteroids[i].base.y;
            float distance = sqrt(dx * dx + dy * dy);

            float speed = ASTEROID_BASE_SPEED;

            // Шанс спавна кометы
            bool isComet = forceComet || (random(100) < (score * 5));
            if (isComet) {
                speed *= COMET_SPEED_MULTIPLIER;
                asteroids[i].isComet = true;
            }
            else {
                asteroids[i].isComet = false;
            }

            asteroids[i].base.vx = (dx / distance) * speed;
            asteroids[i].base.vy = (dy / distance) * speed;
            asteroids[i].base.active = true;
            asteroids[i].size = random(3) + 1; // Размер 1-3

            activeAsteroids++;
            break;
        }
    }
}

// ==================== ПРОВЕРКА КОЛЛИЗИЙ ====================
bool checkCollision(float x1, float y1, float r1, float x2, float y2, float r2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    float distanceSq = dx * dx + dy * dy;
    float radiusSum = r1 + r2;
    return distanceSq < (radiusSum * radiusSum);
}

// ==================== ОПТИМИЗИРОВАННЫЕ ПРОВЕРКИ КОЛЛИЗИЙ ====================
bool checkBulletAsteroidCollision(const Bullet& bullet, const Asteroid& asteroid) {
    if (!bullet.base.active || !asteroid.base.active) return false;

    float dx = bullet.base.x - asteroid.base.x;
    float dy = bullet.base.y - asteroid.base.y;
    float distanceSq = dx * dx + dy * dy;
    float radiusSum = BULLET_COLLISION_RADIUS + (ASTEROID_RADIUS_BASE + asteroid.size);
    return distanceSq < (radiusSum * radiusSum);
}

bool checkShipAsteroidCollision(const Ship& ship, const Asteroid& asteroid) {
    if (!ship.base.active || !asteroid.base.active) return false;

    float dx = ship.base.x - asteroid.base.x;
    float dy = ship.base.y - asteroid.base.y;
    float distanceSq = dx * dx + dy * dy;
    float radiusSum = SHIP_COLLISION_RADIUS + (ASTEROID_RADIUS_BASE + asteroid.size);
    return distanceSq < (radiusSum * radiusSum);
}