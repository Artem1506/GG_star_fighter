#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <Arduino.h>
#include <TFT_eSPI.h>

// ==================== ��������� ====================
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 160

// ������� ��������
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

// ���� ��� ������������
#define TRANSPARENT_COLOR 0xF81F  

// ��� ������� � ������� RGB565 (16-bit)
#define IMAGE_FORMAT_RGB565 0

// ==================== ��������� ������ ====================
enum ScreenState {
    SCREEN_LOGO,
    SCREEN_MENU,
    SCREEN_GAME,
    SCREEN_GAME_OVER
};

// ==================== ������� ������������� ====================
void graphicsInit(TFT_eSPI* display);

// ==================== ����� �������� � ���� ====================
void drawLogo();
void drawStartMenu(uint8_t frame);  // frame � ��� ��������

// �������������� ����������� �������
void drawMenuAnimation();
void clearScreen();

// ==================== ������� ����� ====================
void drawGameBackground();
void drawShip(int x, int y, int angle, bool thrustOn);
void drawBullet(int x, int y);
void drawAsteroid(int x, int y, bool comet);
void drawExplosion(int x, int y, uint8_t frame);
void drawScore(int score);

// ==================== ����� ���������� ====================
void drawGameOver(int score, int highscore);

#endif
