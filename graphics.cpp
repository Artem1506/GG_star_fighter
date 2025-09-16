#include "graphics.h"

// ���������� ��������� �� �������
static TFT_eSPI* tft;

// ==================== ������������� ====================
void graphicsInit(TFT_eSPI* display) {
    tft = display;
    tft->init();
    tft->setRotation(0);
    tft->fillScreen(TFT_BLACK);
}

// ==================== ������� ====================
void drawLogo() {
    tft->fillScreen(TFT_BLACK);

    // �������� ��� ��������
    tft->setTextColor(TFT_YELLOW, TFT_BLACK);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("STAR FIGHTER", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

    // TODO: �������� �� ��������� ������� ��������
}

// ==================== ��������� ���� ====================
void drawStartMenu(uint8_t frame) {
    tft->fillScreen(TFT_BLACK);

    // ���������
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextDatum(TC_DATUM);
    tft->drawString("PRESS TO START", SCREEN_WIDTH / 2, 20);

    // ������� �������� (��� �������� �����)
    if (frame % 20 < 10) {
        tft->drawLine(10, SCREEN_HEIGHT - 30, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 30, TFT_BLUE);
    }
    else {
        tft->drawLine(10, SCREEN_HEIGHT - 30, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 30, TFT_RED);
    }

    // TODO: �������� �� �������� ��������
}

void clearScreen() {
    if (tft) {
        tft->fillScreen(TFT_BLACK);
    }
}

void drawMenuAnimation() {
    if (!tft) return;
    // ���������� "��������" � �������� �����
    static bool toggle = false;
    toggle = !toggle;

    if (toggle) {
        tft->setTextColor(TFT_YELLOW, TFT_BLACK);
        tft->drawString("PRESS BUTTON TO START", 40, 120, 2);
    }
    else {
        tft->setTextColor(TFT_BLACK, TFT_BLACK);
        tft->drawString("PRESS BUTTON TO START", 40, 120, 2);
    }
}


// ==================== ������� ����� ====================
void drawGameBackground() {
    tft->fillScreen(TFT_BLACK);

    // TODO: �������� �� ������� �����������
}

void drawShip(int x, int y, int angle, bool thrustOn) {
    // ��������: ������ ����������� ������ �������
    int size = 10;
    int x1 = x + cos(radians(angle)) * size;
    int y1 = y + sin(radians(angle)) * size;
    int x2 = x + cos(radians(angle + 140)) * size;
    int y2 = y + sin(radians(angle + 140)) * size;
    int x3 = x + cos(radians(angle - 140)) * size;
    int y3 = y + sin(radians(angle - 140)) * size;

    tft->fillTriangle(x1, y1, x2, y2, x3, y3, TFT_WHITE);

    // �������� ���������
    if (thrustOn) {
        int xb = x - cos(radians(angle)) * (size / 2);
        int yb = y - sin(radians(angle)) * (size / 2);
        tft->fillTriangle(x2, y2, x3, y3, xb, yb, TFT_ORANGE);
    }

    // TODO: �������� �� ������� ������� ��� ������
}

void drawBullet(int x, int y) {
    tft->fillCircle(x, y, 2, TFT_YELLOW);
    // TODO: �������� �� ������ ����
}

void drawAsteroid(int x, int y, bool comet) {
    if (!tft) return;

    // ���������� ��������� � ���� ������� ����� ��� ��������/������
    if (comet) {
        // ������ � �������/����
        tft->fillCircle(x, y, 8, TFT_ORANGE);
        tft->drawCircle(x, y, 8, TFT_RED);
    }
    else {
        // ������� �������� � �����
        tft->fillCircle(x, y, 8, TFT_BLUE);
        tft->drawCircle(x, y, 8, TFT_WHITE);
    }

    // TODO: �������� �� ��������/��������� ������� ���������
}

void drawExplosion(int x, int y, uint8_t frame) {
    // ������� �������� � ���� �������������
    int radius = frame;
    tft->drawCircle(x, y, radius, TFT_ORANGE);
    // TODO: �������� �� �������� ������
}

void drawScore(int score) {
    tft->setTextColor(TFT_GREEN, TFT_BLACK);
    tft->setTextDatum(TL_DATUM);
    tft->drawString("SCORE: " + String(score), 2, 2);
}

// ==================== ����� ���������� ====================
void drawGameOver(int score, int highscore) {
    tft->fillScreen(TFT_BLACK);

    tft->setTextColor(TFT_RED, TFT_BLACK);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("GAME OVER", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 20);

    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->drawString("SCORE: " + String(score), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    tft->drawString("BEST: " + String(highscore), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 20);

    tft->setTextColor(TFT_YELLOW, TFT_BLACK);
    tft->drawString("PRESS TO RESTART", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 20);

    // TODO: �������� ������ "Game Over"
}
