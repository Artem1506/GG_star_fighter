#pragma once

// ������� ���������������� ���������� ��� ESP32
constexpr uint8_t GRID_CELL_SIZE = 32; // 128/4 = 32, 160/5 = 32

inline uint8_t getGridIndex(int16_t x, int16_t y) {
    uint8_t gridX = x / GRID_CELL_SIZE;
    uint8_t gridY = y / GRID_CELL_SIZE;
    return gridY * 4 + gridX; // 4 ������� (128/32), 5 ����� (160/32)
}

// ������� ��������������� �������� �� �����
inline bool mightCollide(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t maxDistance) {
    return abs(x1 - x2) < maxDistance && abs(y1 - y2) < maxDistance;
}

void updateOptimizedCollisions() {
    // ��������������� ���������� �� ����� (����� �������)
    uint8_t asteroidGrid[MAX_ASTEROIDS];
    for (uint8_t a = 0; a < asteroidCount; a++) {
        asteroidGrid[a] = getGridIndex(asteroids[a].base.x, asteroids[a].base.y);
    }

    uint8_t bulletGrid[MAX_BULLETS];
    for (uint8_t b = 0; b < bulletCount; b++) {
        bulletGrid[b] = getGridIndex(bullets[b].base.x, bullets[b].base.y);
    }

    uint8_t shipGrid = getGridIndex(playerShip.base.x, playerShip.base.y);

    // ���������������� ��������
    for (uint8_t b = 0; b < bulletCount; b++) {
        if (!bullets[b].base.active) continue;

        for (uint8_t a = 0; a < asteroidCount; a++) {
            if (!asteroids[a].base.active) continue;

            // ������� ��������������� �������� �� �����
            if (abs(bulletGrid[b] - asteroidGrid[a]) > 6) continue; // �������� ������

            // ������� �������� ����������
            if (!mightCollide(bullets[b].base.x, bullets[b].base.y,
                asteroids[a].base.x, asteroids[a].base.y,
                ASTEROID_RADIUS_BASE + 20)) continue;

            // ������ ��������
            if (checkBulletAsteroidCollision(
                bullets[b].base.x, bullets[b].base.y,
                asteroids[a].base.x, asteroids[a].base.y,
                asteroids[a].size)) {

                handleBulletAsteroidCollision(b, a);
                break;
            }
        }
    }
}
