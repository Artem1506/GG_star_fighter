#ifndef GRAPHICS_H
#define GRAPHICS_H

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

#endif
