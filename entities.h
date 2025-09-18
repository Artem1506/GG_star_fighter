#ifndef ENTITIES_H
#define ENTITIES_H

#include <Arduino.h>
#include "input.h"

// ==================== ��������� ====================
constexpr uint8_t MAX_BULLETS = 10;
constexpr uint8_t MAX_ASTEROIDS = 20;
constexpr uint32_t BULLET_DELAY = 300;   // ms
constexpr float SHIP_SPEED = 2.0f;
constexpr float BULLET_SPEED = 4.0f;
constexpr float ASTEROID_BASE_SPEED = 1.0f;
constexpr float COMET_SPEED_MULTIPLIER = 1.5f;

// ==================== ������� ��������� ====================
struct Entity {
    float x, y;        // �������
    float vx, vy;      // ��������
    bool active;       // ���� ����������
};

// ==================== ������� ====================
struct Ship {
    Entity base;
    int16_t rotation;   // ���� (0-359 ��������)
    bool boosting;      // ���������
    uint32_t lastShot;  // ����� ���������� ��������
};

// ==================== ���� ====================
struct Bullet {
    Entity base;
    uint32_t spawnTime; // ����� ��������
};

// ==================== �������� ====================
struct Asteroid {
    Entity base;
    uint8_t size;       // ������
    bool isComet;       // ���� ������
};

// ==================== ��������� ====================
void initEntities();
void resetEntities();

Ship* getPlayerShip();
Bullet* getBullets();
Asteroid* getAsteroids();

uint8_t getActiveBullets();
uint8_t getActiveAsteroids();

void updateShip(Ship* ship, const InputState& input);
bool fireBullet(Ship* ship);
void updateBullets();
void updateAsteroids(uint16_t score);
void spawnAsteroid(uint16_t score, bool forceComet = false);

bool checkCollision(float x1, float y1, float r1, float x2, float y2, float r2);
bool checkBulletAsteroidCollision(const Bullet& bullet, const Asteroid& asteroid);
bool checkShipAsteroidCollision(const Ship& ship, const Asteroid& asteroid);

// ==================== ��������� �������� �������� ====================
constexpr float SHIP_COLLISION_RADIUS = 6.0f;
constexpr float BULLET_COLLISION_RADIUS = 2.0f;
constexpr float ASTEROID_RADIUS_BASE = 4.0f;

bool checkBulletAsteroidCollision(const Bullet& bullet, const Asteroid& asteroid);
bool checkShipAsteroidCollision(const Ship& ship, const Asteroid& asteroid);

#endif
