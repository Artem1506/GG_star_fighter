#include "entities.h"

// ==================== Глобальные переменные ====================
static Ship player;
static Bullet bullets[MAX_BULLETS];
static Asteroid asteroids[MAX_ASTEROIDS];

static unsigned long lastBulletTime = 0;

// ==================== Инициализация ====================
void initEntities() {
    resetEntities();
}

void resetEntities() {
    player.x = 64;
    player.y = 32;
    player.angle = 0;
    player.alive = true;

    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
    for (int i = 0; i < MAX_ASTEROIDS; i++) asteroids[i].active = false;

    lastBulletTime = 0;
}

// ==================== Доступ ====================
Ship* getShip() { return &player; }
Bullet* getBullets() { return bullets; }
Asteroid* getAsteroids() { return asteroids; }

// ==================== Обновление корабля ====================
void updateShip(bool moveForward) {
    if (!player.alive) return;

    if (moveForward) {
        player.x += SHIP_SPEED * cos(radians(player.angle));
        player.y += SHIP_SPEED * sin(radians(player.angle));
    }

    // Зацикливание на краях экрана (wrap-around)
    if (player.x < 0) player.x = 127;
    if (player.x > 127) player.x = 0;
    if (player.y < 0) player.y = 63;
    if (player.y > 63) player.y = 0;
}

// ==================== Пули ====================
bool fireBullet(int angle) {
    if (millis() - lastBulletTime < BULLET_DELAY) return false;

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].x = player.x;
            bullets[i].y = player.y;
            bullets[i].dx = BULLET_SPEED * cos(radians(angle));
            bullets[i].dy = BULLET_SPEED * sin(radians(angle));
            bullets[i].active = true;
            bullets[i].spawnTime = millis();
            lastBulletTime = millis();
            return true;
        }
    }
    return false;
}

void updateBullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].x += bullets[i].dx;
            bullets[i].y += bullets[i].dy;

            if (millis() - bullets[i].spawnTime > 2000) {
                bullets[i].active = false;
            }
        }
    }
}

// ==================== Астероиды ====================
void updateAsteroids(unsigned long score) {
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].active) {
            asteroids[i].x += asteroids[i].dx;
            asteroids[i].y += asteroids[i].dy;

            // Зацикливание
            if (asteroids[i].x < 0) asteroids[i].x = 127;
            if (asteroids[i].x > 127) asteroids[i].x = 0;
            if (asteroids[i].y < 0) asteroids[i].y = 63;
            if (asteroids[i].y > 63) asteroids[i].y = 0;
        }
    }

    // Автоспавн новых астероидов при росте счета
    int targetCount = 1 + score / 5;
    int currentCount = 0;
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].active) currentCount++;
    }
    if (currentCount < targetCount) {
        for (int i = 0; i < MAX_ASTEROIDS; i++) {
            if (!asteroids[i].active) {
                asteroids[i].x = random(128);
                asteroids[i].y = random(64);

                float angle = random(360) * PI / 180.0f;
                float speed = ASTEROID_BASE_SPEED;

                // Вероятность появления кометы
                bool comet = random(100) < (score * 5);
                if (comet) speed *= 1.5;

                asteroids[i].dx = cos(angle) * speed;
                asteroids[i].dy = sin(angle) * speed;
                asteroids[i].active = true;
                asteroids[i].comet = comet;
                break;
            }
        }
    }
}

// ==================== Коллизии ====================
bool checkBulletHitAsteroid(int bulletIdx, int asteroidIdx) {
    if (!bullets[bulletIdx].active || !asteroids[asteroidIdx].active) return false;

    if (abs(bullets[bulletIdx].x - asteroids[asteroidIdx].x) < 3 &&
        abs(bullets[bulletIdx].y - asteroids[asteroidIdx].y) < 3) {
        bullets[bulletIdx].active = false;
        asteroids[asteroidIdx].active = false;
        return true;
    }
    return false;
}

bool checkShipCollision(int asteroidIdx) {
    if (!player.alive || !asteroids[asteroidIdx].active) return false;

    if (abs(player.x - asteroids[asteroidIdx].x) < 4 &&
        abs(player.y - asteroids[asteroidIdx].y) < 4) {
        player.alive = false;
        return true;
    }
    return false;
}
