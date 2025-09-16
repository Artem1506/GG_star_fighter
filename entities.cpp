#include "entities.h"
#include "game.h"
#include "graphics.h"
#include "audio.h"
#include <random>

extern AnimationManager animationManager;
extern AudioManager audio;

// ==================== Глобальные переменные ====================
static Ship player;
static Bullet bullets[MAX_BULLETS];
static Asteroid asteroids[MAX_ASTEROIDS];

static unsigned long lastBulletTime = 0;

void handleBulletAsteroidCollision(Bullet& bullet, Asteroid& asteroid) {
    // Создаем анимацию взрыва
    animationManager.createExplosion(asteroid.position);

    // Проигрываем звук
    audio.playHit();

    // Уничтожаем объекты
    bullet.active = false;
    asteroid.active = false;

    // Добавляем очки
    game.addScore(1);
}

void handleShipAsteroidCollision(Ship& ship, Asteroid& asteroid) {
    // Анимация взрыва корабля
    animationManager.createShipExplosion(ship.position);

    // Звук столкновения
    audio.playCrash();

    // Переход в состояние game over
    game.changeState(STATE_GAME_OVER);
}

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

void Comet::update() {
    // Движение на 50% быстрее астероида
    position.x += velocity.x * 1.5f;
    position.y += velocity.y * 1.5f;

    // Выбор спрайта на основе направления
    directionSpriteIndex = (int)(atan2(velocity.y, velocity.x) * 180 / PI) / 10;
    directionSpriteIndex = (directionSpriteIndex + 36) % 36; // нормализация
}

void SpawnManager::update(int playerScore) {
    // Определяем целевое количество астероидов
    int targetCount = 1 + (playerScore / 5);

    // Спавним новые астероиды/кометы если нужно
    int currentCount = asteroids.size();
    if (currentCount < targetCount) {
        // Вероятность кометы = счет * 5%
        if (rand() % 100 < playerScore * 5) {
            spawnComet();
        }
        else {
            spawnAsteroid();
        }
    }
}

void SpawnManager::spawnComet() {
    Comet* comet = new Comet();
    comet->position = getSpawnPositionOutsideScreen();
    comet->velocity = getRandomDirection() * (2.0f * 1.5f); // +50% скорости
    comet->directionSpriteIndex = rand() % 17; // случайный спрайт

    // Вычисляем правильный спрайт на основе направления движения
    float angle = atan2(comet->velocity.y, comet->velocity.x);
    comet->directionSpriteIndex = static_cast<int>((angle * 180 / M_PI) / 10) % 17;

    asteroids.push_back(comet);
}

Vector2D SpawnManager::getSpawnPositionOutsideScreen() {
    // Спавн за пределами экрана (0-3: top, right, bottom, left)
    int side = rand() % 4;
    switch (side) {
    case 0: return Vector2D(rand() % SCREEN_WIDTH, -20); // top
    case 1: return Vector2D(SCREEN_WIDTH + 20, rand() % SCREEN_HEIGHT); // right
    case 2: return Vector2D(rand() % SCREEN_WIDTH, SCREEN_HEIGHT + 20); // bottom
    case 3: return Vector2D(-20, rand() % SCREEN_HEIGHT); // left
    }
    return Vector2D(0, 0);
}
