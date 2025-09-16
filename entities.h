#ifndef ENTITIES_H
#define ENTITIES_H

#include <Arduino.h>

// ==================== ������� ====================
struct Ship {
    float x, y;
    int angle;       // ���� (�������)
    bool alive;
};

// ==================== ���� ====================
struct Bullet {
    float x, y;
    float dx, dy;
    bool active;
    unsigned long spawnTime;
};

// ==================== �������� ====================
struct Asteroid {
    float x, y;
    float dx, dy;
    bool active;
    int comet;      // ������� ��������
};

// ==================== ��������� ====================
#define MAX_BULLETS   10
#define MAX_ASTEROIDS 20
#define BULLET_DELAY  300   // ��
#define SHIP_SPEED    2.0f
#define BULLET_SPEED  4.0f
#define ASTEROID_BASE_SPEED 1.0f

// ==================== ��������� ====================
void initEntities();
void resetEntities();

Ship* getShip();
Bullet* getBullets();
Asteroid* getAsteroids();

void updateShip(bool moveForward);
bool fireBullet(int angle);  // true ���� ������� �������
void updateBullets();
void updateAsteroids(unsigned long score);

bool checkBulletHitAsteroid(int bulletIdx, int asteroidIdx);
bool checkShipCollision(int asteroidIdx);

#endif
