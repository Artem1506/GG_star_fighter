#ifndef GRAPHICS_H
#define GRAPHICS_H

#pragma once
#include "Vector2D.h"
#include <vector>
#include <Arduino.h>
#include <TFT_eSPI.h>

// ==================== Константы ====================
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 160

// Размеры спрайтов
#define LOGO_WIDTH   128
#define LOGO_HEIGHT  160
#define BG_WIDTH     128
#define BG_HEIGHT    160
#define NAME_WIDTH   117
#define NAME_HEIGHT  48
#define PRESS_WIDTH  48
#define PRESS_HEIGHT 15
#define MAIN_BG_WIDTH 128
#define MAIN_BG_HEIGHT 160

// Цвет для прозрачности
#define TRANSPARENT_COLOR 0xF81F  

// Все спрайты в формате RGB565 (16-bit)
#define IMAGE_FORMAT_RGB565 0

// ==================== Состояния экрана ====================
enum ScreenState {
    SCREEN_LOGO,
    SCREEN_MENU,
    SCREEN_GAME,
    SCREEN_GAME_OVER
};

// ==================== Функции инициализации ====================
void graphicsInit(TFT_eSPI* display);

// ==================== Экран логотипа и меню ====================
void drawLogo();
void drawStartMenu(uint8_t frame);  // frame — для анимации

// Дополнительные графические функции
void drawMenuAnimation();
void clearScreen();

// ==================== Игровая сцена ====================
void drawGameBackground();
void drawShip(int x, int y, int angle, bool thrustOn);
void drawBullet(int x, int y);
void drawAsteroid(int x, int y, bool comet);
void drawExplosion(int x, int y, uint8_t frame);
void drawScore(int score);

// ==================== Экран завершения ====================
void drawGameOver(int score, int highscore);

class Graphics {
public:
    bool init();
    void drawSprite(int16_t x, int16_t y, const uint8_t* bitmap);
    void clear();
    void present();

    // Константы экрана (настройте под ваш дисплей)
    static const int16_t SCREEN_WIDTH = 240;
    static const int16_t SCREEN_HEIGHT = 320;
};

class Animation {
public:
    void init(const char* spriteSheetPath, int frameCount, float frameDuration);
    void startAt(const Vector2D& position);
    void update();
    void draw();
    bool isFinished() const;
    bool isActive() const;

private:
    std::vector<const char*> frames;
    Vector2D position;
    int currentFrame;
    unsigned long startTime;
    unsigned long lastFrameTime;
    float durationPerFrame;
    bool active;
};

class AnimationManager {
public:
    void update();
    void render();
    void createExplosion(const Vector2D& position);
    void createShipExplosion(const Vector2D& position);
    void createBulletImpact(const Vector2D& position);

    std::vector<Animation> activeAnimations;
};

#endif
