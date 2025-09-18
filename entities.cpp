#include "entities.h"
#include "game.h"
#include "graphics.h"
#include "audio.h"
#include <random>
#include <cmath>

extern AnimationManager animationManager;
extern AudioManager audio;

// ==================== Глобальные переменные ====================
static Ship player;
static Bullet bullets[MAX_BULLETS];
static Asteroid asteroids[MAX_ASTEROIDS];

const float DEG_PER_RAD = 180.0f / M_PI;
const int SPRITE_COUNT = 17;
const float ANGLE_STEP = 360.0f / SPRITE_COUNT;

// Предварительно вычисленная таблица косинусов (0-360 градусов с шагом 10)
const int8_t COS_TABLE[36] = {
    100, 98, 94, 87, 77, 64, 50, 34, 17, 0,
    -17, -34, -50, -64, -77, -87, -94, -98, -100,
    -98, -94, -87, -77, -64, -50, -34, -17, 0,
    17, 34, 50, 64, 77, 87, 94, 98
};

// Альтернативный способ с предварительным расчетом
int Comet::calculateSpriteIndexOptimized() const {
    // Использование предварительно рассчитанной таблицы
    static const float angleThresholds[] = {
        10.0f, 30.0f, 50.0f, 70.0f, 90.0f, 110.0f, 130.0f,
        150.0f, 170.0f, 190.0f, 210.0f, 230.0f, 250.0f,
        270.0f, 290.0f, 310.0f, 330.0f
    };

    float angle = atan2(velocity.y, velocity.x) * DEG_PER_RAD;
    if (angle < 0) angle += 360.0f;

    for (int i = 0; i < SPRITE_COUNT; ++i) {
        if (angle < angleThresholds[i]) {
            return i;
        }
    }

    return 0;
}

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
void updateShip(Ship* ship, float thrustPower) {
    // БЫСТРЫЙ расчет thrust через таблицу
    int angleIndex = (ship->rotation % 360) / 10;
    if (angleIndex < 0) angleIndex += 36;

    float thrustX = COS_TABLE[angleIndex] * 0.01f * thrustPower;
    float thrustY = COS_TABLE[(angleIndex + 9) % 36] * 0.01f * thrustPower;

    ship->base.vx += thrustX;
    ship->base.vy += thrustY;

    // Обновление позиции
    ship->base.x += ship->base.vx;
    ship->base.y += ship->base.vy;

    // СУПЕР-БЫСТРАЯ обработка границ
    if (ship->base.x < 0) ship->base.x = Graphics::SCREEN_WIDTH;
    else if (ship->base.x > Graphics::SCREEN_WIDTH) ship->base.x = 0;

    if (ship->base.y < 0) ship->base.y = Graphics::SCREEN_HEIGHT;
    else if (ship->base.y > Graphics::SCREEN_HEIGHT) ship->base.y = 0;
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

bool checkCollision(float x1, float y1, float r1, float x2, float y2, float r2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return (dx * dx + dy * dy) < ((r1 + r2) * (r1 + r2));
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

void Ship::update() {
    // Вместо position.x += velocity.x;
    x += velocityX;
    y += velocityY;

    // Обработка границ экрана
    if (x < 0) x = DISPLAY_WIDTH;
    if (x > DISPLAY_WIDTH) x = 0;
    if (y < 0) y = DISPLAY_HEIGHT;
    if (y > DISPLAY_HEIGHT) y = 0;
}

void Ship::applyThrust() {
    // Простой расчет thrust вместо Vector2D
    float thrustX = cos(rotation * M_PI / 180.0) * thrustPower;
    float thrustY = sin(rotation * M_PI / 180.0) * thrustPower;

    velocityX += thrustX;
    velocityY += thrustY;
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

void SpawnManager::spawnAsteroid() {
    auto asteroid = std::make_unique<Asteroid>();
    // Инициализация asteroid
    asteroids.push_back(std::move(asteroid));
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

void SpawnManager::clearAll() {
    asteroids.clear();  // Автоматическое освобождение памяти
    comets.clear();     // Автоматическое освобождение памяти
}
