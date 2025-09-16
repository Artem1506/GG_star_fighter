#ifndef ENTITIES_H
#define ENTITIES_H

#pragma once
#include <vector>
#include "Vector2D.h"
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

class Asteroid {
public:
    virtual void update();
    virtual void draw();
    virtual void destroy();

    Vector2D position;
    Vector2D velocity;
    float size;
    bool active;
};

class Comet : public Asteroid {
public:
    Comet();
    void update() override;
    void draw() override;

private:
    int directionSpriteIndex; // ������ ������� �� 17 ���������
};

class Ship {
public:
    void update();
    void draw();
    void applyThrust();
    void shoot();

    Vector2D position;
    Vector2D velocity;
    float rotation; // � ��������
    float thrustPower;

    bool isBoosting;
    unsigned long lastShotTime;
};

class SpawnManager {
public:
    void init();
    void update(int playerScore);
    void spawnAsteroid();
    void spawnComet();
    void clearAll();

    std::vector<Asteroid*> asteroids;

private:
    Vector2D getSpawnPositionOutsideScreen();
    Vector2D getRandomDirection();
    unsigned long lastSpawnTime;
};

#endif
