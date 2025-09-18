#ifndef ENTITIES_H
#define ENTITIES_H

#pragma once
#include <memory>
#include <vector>
#include <Arduino.h>

// ==================== Корабль ====================
struct Ship {
    Entity base;       // базовая структура
    bool boosting;     // ускорение
    uint16_t lastShot; // время выстрела (в ms/10 для экономии)
    int8_t rotation;   // угол (-128 to 127 градусов)
};

// ==================== Пуля ====================
struct Bullet {
    Entity base;
    uint16_t spawnTime; // время создания
};

// ==================== Астероид ====================
struct Asteroid {
    Entity base;       // базовая структура  
    uint8_t size;      // размер (0-255)
};

struct Entity {
    float x, y;        // позиция
    float vx, vy;      // скорость
    uint8_t type;      // тип объекта
    bool active;       // флаг активности
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

class Asteroid : public Entity {
public:
    void update();
    void draw();
};

class Bullet : public Entity {
public:
    void update();
    void draw();
    unsigned long spawnTime;
};

class Comet : public Asteroid {
public:
    Comet();
    void update() override;
    void draw() override;

private:
    int directionSpriteIndex; // индекс спрайта из 17 вариантов
};

class Ship : public Entity {
public:
    void update();
    void draw();
    void applyThrust();
    void shoot();

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
    std::vector<std::unique_ptr<Asteroid>> asteroids;
    std::vector<std::unique_ptr<Comet>> comets;

private:
    Vector2D getSpawnPositionOutsideScreen();
    Vector2D getRandomDirection();
    unsigned long lastSpawnTime;
};

#endif
