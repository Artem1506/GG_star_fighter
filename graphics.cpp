#include "graphics.h"
#include "storage.h"

extern StorageManager storage;

// Глобальный указатель на дисплей
static TFT_eSPI* tft;

// ==================== Инициализация ====================
void graphicsInit(TFT_eSPI* display) {
    tft = display;
    tft->init();
    tft->setRotation(0);
    tft->fillScreen(TFT_BLACK);
}

// ==================== Логотип ====================
void drawLogo() {
    tft->fillScreen(TFT_BLACK);

    // Заглушка для логотипа
    tft->setTextColor(TFT_YELLOW, TFT_BLACK);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("STAR FIGHTER", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

    // TODO: заменить на отрисовку спрайта логотипа
}

// ==================== Стартовое меню ====================
void drawStartMenu(uint8_t frame) {
    tft->fillScreen(TFT_BLACK);

    // Заголовок
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextDatum(TC_DATUM);
    tft->drawString("PRESS TO START", SCREEN_WIDTH / 2, 20);

    // Простая анимация (две мигающие линии)
    if (frame % 20 < 10) {
        tft->drawLine(10, SCREEN_HEIGHT - 30, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 30, TFT_BLUE);
    }
    else {
        tft->drawLine(10, SCREEN_HEIGHT - 30, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 30, TFT_RED);
    }

    // TODO: заменить на анимацию спрайтов
}

void clearScreen() {
    if (tft) {
        tft->fillScreen(TFT_BLACK);
    }
}

void drawMenuAnimation() {
    if (!tft) return;
    // Простейшая "анимация" — мигающий текст
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


// ==================== Игровая сцена ====================
void drawGameBackground() {
    tft->fillScreen(TFT_BLACK);

    // TODO: заменить на фоновое изображение
}

void drawShip(int x, int y, int angle, bool thrustOn) {
    // Заглушка: рисуем треугольник вместо корабля
    int size = 10;
    int x1 = x + cos(radians(angle)) * size;
    int y1 = y + sin(radians(angle)) * size;
    int x2 = x + cos(radians(angle + 140)) * size;
    int y2 = y + sin(radians(angle + 140)) * size;
    int x3 = x + cos(radians(angle - 140)) * size;
    int y3 = y + sin(radians(angle - 140)) * size;

    tft->fillTriangle(x1, y1, x2, y2, x3, y3, TFT_WHITE);

    // Анимация двигателя
    if (thrustOn) {
        int xb = x - cos(radians(angle)) * (size / 2);
        int yb = y - sin(radians(angle)) * (size / 2);
        tft->fillTriangle(x2, y2, x3, y3, xb, yb, TFT_ORANGE);
    }

    // TODO: заменить на спрайты корабля под углами
}

void drawBullet(int x, int y) {
    tft->fillCircle(x, y, 2, TFT_YELLOW);
    // TODO: заменить на спрайт пули
}

void drawAsteroid(int x, int y, bool comet) {
    if (!tft) return;

    // Простейшая отрисовка — круг разного цвета для обычного/кометы
    if (comet) {
        // комета — краснее/ярче
        tft->fillCircle(x, y, 8, TFT_ORANGE);
        tft->drawCircle(x, y, 8, TFT_RED);
    }
    else {
        // обычный астероид — серый
        tft->fillCircle(x, y, 8, TFT_BLUE);
        tft->drawCircle(x, y, 8, TFT_WHITE);
    }

    // TODO: заменить на загрузку/рисование спрайта астероида
}

void drawExplosion(int x, int y, uint8_t frame) {
    // Простая анимация — круг увеличивается
    int radius = frame;
    tft->drawCircle(x, y, radius, TFT_ORANGE);
    // TODO: заменить на анимацию взрыва
}

void drawScore(int score) {
    tft->setTextColor(TFT_GREEN, TFT_BLACK);
    tft->setTextDatum(TL_DATUM);
    tft->drawString("SCORE: " + String(score), 2, 2);
}

// ==================== Экран завершения ====================
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


    void Animation::init(const char* spriteSheetPath, int frameCount, float frameDuration) {
        // Загрузка спрайтов из SD-карты
        for (int i = 0; i < frameCount; i++) {
            char framePath[50];
            sprintf(framePath, "%s_%d.bin", spriteSheetPath, i);
            frames.push_back(strdup(framePath));
        }
        this->durationPerFrame = frameDuration;
    }

    void Animation::startAt(const Vector2D & pos) {
        position = pos;
        currentFrame = 0;
        startTime = millis();
        lastFrameTime = startTime;
        active = true;
    }

    void Animation::update() {
        if (!active) return;

        unsigned long currentTime = millis();
        if (currentTime - lastFrameTime >= durationPerFrame * 1000) {
            currentFrame++;
            lastFrameTime = currentTime;

            if (currentFrame >= frames.size()) {
                active = false;
            }
        }
    }

    void AnimationManager::createExplosion(const Vector2D & position) {
        Animation explosion;
        explosion.init("/spr_explosion", 8, 0.1f); // 8 кадров, 0.1 сек на кадр
        explosion.startAt(position);
        activeAnimations.push_back(explosion);
    }

    void AnimationManager::createShipExplosion(const Vector2D & position) {
        Animation explosion;
        explosion.init("/spr_ship_explosion", 12, 0.15f); // 12 кадров, 0.15 сек на кадр
        explosion.startAt(position);
        activeAnimations.push_back(explosion);
    }
    // TODO: добавить спрайт "Game Over"
}
