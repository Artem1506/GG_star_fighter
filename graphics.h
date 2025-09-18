#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <Arduino.h>

// ==================== Константы экрана ====================
constexpr int16_t SCREEN_WIDTH = 128;
constexpr int16_t SCREEN_HEIGHT = 160;

// ==================== Константы размеров спрайтов ====================
constexpr uint8_t SHIP_WIDTH = 17;
constexpr uint8_t SHIP_HEIGHT = 17;
constexpr uint8_t BULLET_WIDTH = 4;
constexpr uint8_t BULLET_HEIGHT = 4;
constexpr uint8_t ASTEROID_WIDTH = 9;
constexpr uint8_t ASTEROID_HEIGHT = 10;
constexpr uint8_t COMET_WIDTH = 24;
constexpr uint8_t COMET_HEIGHT = 12;
constexpr uint8_t EXPLOSION_WIDTH = 24;
constexpr uint8_t EXPLOSION_HEIGHT = 23;

constexpr uint8_t LOGO_WIDTH = 128;
constexpr uint8_t LOGO_HEIGHT = 160;
constexpr uint8_t BG_WIDTH = 128;
constexpr uint8_t BG_HEIGHT = 160;
constexpr uint8_t NAME_WIDTH = 117;
constexpr uint8_t NAME_HEIGHT = 48;
constexpr uint8_t PRESS_WIDTH = 48;
constexpr uint8_t PRESS_HEIGHT = 15;
constexpr uint8_t MAIN_BG_WIDTH = 128;
constexpr uint8_t MAIN_BG_HEIGHT = 160;

// ==================== Упрощенные структуры ====================
struct Entity {
    int16_t x, y;
    int8_t vx, vy;
    bool active;
};

// ==================== Основной класс Graphics ====================
class Graphics {
public:
    bool init();
    void drawSprite(int16_t x, int16_t y, const uint8_t* bitmap, uint8_t w, uint8_t h, uint8_t bpp = 1);
    void clear();
    void present();

    // Основные функции отрисовки
    void drawLogo();
    void drawStartMenu(uint8_t frame);
    void drawGameBackground();
    void drawScore(uint16_t score);
    void drawGameOver(uint16_t score, uint16_t highScore);

    // Оптимизированные функции для игровых объектов
    void drawShip(int16_t x, int16_t y, uint16_t angle, bool isBoosting = false, uint8_t boostFrame = 0);
    void drawBullet(int16_t x, int16_t y, uint16_t angle);
    void drawAsteroid(int16_t x, int16_t y, uint8_t size);
    void drawComet(int16_t x, int16_t y, uint16_t direction);
    void drawExplosion(int16_t x, int16_t y, uint8_t frame);

    // Утилиты для центрирования
    static int16_t centerX(int16_t x, uint8_t width) { return x - width / 2; }
    static int16_t centerY(int16_t y, uint8_t height) { return y - height / 2; }

private:
    // Быстрая загрузка спрайтов
    const uint8_t* loadSprite(const char* filename);

    // Вспомогательные функции
    bool isSpriteVisible(int16_t x, int16_t y, uint8_t w, uint8_t h);
    void drawPixel(int16_t x, int16_t y, uint8_t color);
};

// ==================== Упрощенная система анимаций ====================
class Animation {
public:
    void start(int16_t x, int16_t y, uint8_t type);
    void update();
    void draw();
    bool isActive() const { return active; }

private:
    int16_t x, y;
    uint8_t type;
    uint8_t frame;
    uint32_t startTime;
    bool active;
};

// ==================== Простой менеджер анимаций ====================
class AnimationManager {
public:
    void update();
    void draw();
    void createExplosion(int16_t x, int16_t y);

private:
    static const uint8_t MAX_ANIMATIONS = 5;
    Animation animations[MAX_ANIMATIONS];
    uint8_t animationCount = 0;
};

#endif