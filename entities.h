#ifndef ENTITIES_H
#define ENTITIES_H

#include <Arduino.h>

// ==================== Корабль ====================
struct Ship {
    float x, y;
    int angle;       // угол (градусы)
    bool alive;
};

// ==================== Пуля ====================
struct Bullet {
    float x, y;
    float dx, dy;
    bool active;
    unsigned long spawnTime;
};

// ==================== Астероид ====================
struct Asteroid {
    float x, y;
    float dx, dy;
    bool active;
    int comet;      // быстрый астероид
};

// ==================== Константы ====================
#define MAX_BULLETS   10
#define MAX_ASTEROIDS 20
#define BULLET_DELAY  300   // мс
#define SHIP_SPEED    2.0f
#define BULLET_SPEED  4.0f
#define ASTEROID_BASE_SPEED 1.0f

// ==================== Интерфейс ====================
void initEntities();
void resetEntities();

Ship* getShip();
Bullet* getBullets();
Asteroid* getAsteroids();

void updateShip(bool moveForward);
bool fireBullet(int angle);  // true если выстрел успешен
void updateBullets();
void updateAsteroids(unsigned long score);

bool checkBulletHitAsteroid(int bulletIdx, int asteroidIdx);
bool checkShipCollision(int asteroidIdx);

#endif
