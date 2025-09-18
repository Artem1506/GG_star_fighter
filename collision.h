#pragma once

// ПРЕДВЫЧИСЛЕННЫЕ РАДИУСЫ КОЛЛИЗИЙ (в пикселях)
constexpr uint8_t SHIP_COLLISION_RADIUS = 6;    // Радиус корабля
constexpr uint8_t BULLET_COLLISION_RADIUS = 2;  // Радиус пули
constexpr uint8_t ASTEROID_RADIUS_BASE = 8;     // Базовый радиус астероида

// БЫСТРАЯ ПРОВЕРКА КОЛЛИЗИЙ МЕЖДУ ДВУМЯ ОБЪЕКТАМИ
inline bool checkCollision(int16_t x1, int16_t y1, uint8_t r1,
    int16_t x2, int16_t y2, uint8_t r2) {
    int16_t dx = x1 - x2;
    int16_t dy = y1 - y2;
    uint16_t distSq = dx * dx + dy * dy;
    uint8_t radSum = r1 + r2;
    return distSq < (radSum * radSum);
}

// СУПЕР-БЫСТРАЯ ВЕРСИЯ ДЛЯ ИЗВЕСТНЫХ ТИПОВ ОБЪЕКТОВ
inline bool checkShipAsteroidCollision(int16_t shipX, int16_t shipY,
    int16_t asteroidX, int16_t asteroidY, uint8_t asteroidSize) {
    int16_t dx = shipX - asteroidX;
    int16_t dy = shipY - asteroidY;
    uint16_t distSq = dx * dx + dy * dy;
    uint8_t radSum = SHIP_COLLISION_RADIUS + (ASTEROID_RADIUS_BASE + asteroidSize);
    return distSq < (radSum * radSum);
}

inline bool checkBulletAsteroidCollision(int16_t bulletX, int16_t bulletY,
    int16_t asteroidX, int16_t asteroidY, uint8_t asteroidSize) {
    int16_t dx = bulletX - asteroidX;
    int16_t dy = bulletY - asteroidY;
    uint16_t distSq = dx * dx + dy * dy;
    uint8_t radSum = BULLET_COLLISION_RADIUS + (ASTEROID_RADIUS_BASE + asteroidSize);
    return distSq < (radSum * radSum);
}
