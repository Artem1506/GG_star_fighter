#include "graphics.h"
#include <SD_MMC.h>
#include <TFT_eSPI.h>

// ==================== Глобальные переменные ====================
static TFT_eSPI* tft = nullptr;
static CachedSprite spriteCache[MAX_CACHED_SPRITES];
static uint8_t spriteCacheCount = 0;

// ==================== Инициализация ====================
bool Graphics::init() {
    // Инициализация дисплея
    tft = new TFT_eSPI();
    tft->init();
    tft->setRotation(0);
    tft->fillScreen(TFT_BLACK);

    // Инициализация SD_MMC
    if (!SD_MMC.begin("/sdcard", true)) { // 1-bit режим для экономии пинов
        Serial.println("SD_MMC initialization failed!");
        return false;
    }

    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return false;
    }

    Serial.print("SD_MMC Card Type: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD) {
        Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
    }
    else {
        Serial.println("UNKNOWN");
    }

    // Предзагрузка критичных спрайтов
    preloadEssentialSprites();

    return true;
}

// ==================== Предзагрузка важных спрайтов ====================
void Graphics::preloadEssentialSprites() {
    const char* essentialSprites[] = {
        "/sprites/logo.bin",
        "/sprites/bg_menu.bin",
        "/sprites/bg_game.bin",
        "/sprites/bg_gameover.bin",
        "/sprites/ship/s_000.bin",
        "/sprites/ship/b_000_1.bin"
    };

    for (const char* filename : essentialSprites) {
        loadSprite(filename);
    }
}

// ==================== Оптимизированная загрузка спрайтов ====================
const uint8_t* Graphics::loadSprite(const char* filename) {
    // Поиск в кэше
    for (uint8_t i = 0; i < spriteCacheCount; i++) {
        if (strcmp(spriteCache[i].filename, filename) == 0) {
            spriteCache[i].lastUsed = millis();
            return spriteCache[i].data;
        }
    }

    // Освобождение места в кэше если нужно
    if (spriteCacheCount >= MAX_CACHED_SPRITES) {
        cleanupSpriteCache();
    }

    // Открытие файла через SD_MMC
    File file = SD_MMC.open(filename, FILE_READ);
    if (!file) {
        Serial.print("Failed to open: ");
        Serial.println(filename);
        return nullptr;
    }

    // Автоопределение параметров спрайта
    uint8_t width = 0, height = 0, bpp = 4;

    if (strstr(filename, "ship")) {
        width = SHIP_WIDTH; height = SHIP_HEIGHT;
        if (strstr(filename, "boost")) bpp = 4; else bpp = 4;
    }
    else if (strstr(filename, "bullet")) {
        width = BULLET_WIDTH; height = BULLET_HEIGHT; bpp = 1;
    }
    else if (strstr(filename, "comet")) {
        width = COMET_WIDTH; height = COMET_HEIGHT; bpp = 4;
    }
    else if (strstr(filename, "explosion")) {
        width = EXPLOSION_WIDTH; height = EXPLOSION_HEIGHT; bpp = 4;
    }
    else if (strstr(filename, "asteroid")) {
        width = ASTEROID_WIDTH; height = ASTEROID_HEIGHT; bpp = 4;
    }
    else if (strstr(filename, "logo")) {
        width = LOGO_WIDTH; height = LOGO_HEIGHT; bpp = 4;
    }
    else if (strstr(filename, "BG")) {
        width = BG_WIDTH; height = BG_HEIGHT; bpp = 4;
    }

    size_t expectedSize = (width * height * bpp) / 8;
    if (file.size() != expectedSize) {
        Serial.print("Size mismatch: ");
        Serial.println(filename);
        file.close();
        return nullptr;
    }

    // Чтение данных
    uint8_t* data = new uint8_t[expectedSize];
    if (file.read(data, expectedSize) != expectedSize) {
        Serial.print("Read error: ");
        Serial.println(filename);
        delete[] data;
        file.close();
        return nullptr;
    }
    file.close();

    // Сохранение в кэш
    if (spriteCacheCount < MAX_CACHED_SPRITES) {
        strncpy(spriteCache[spriteCacheCount].filename, filename, MAX_FILENAME_LENGTH);
        spriteCache[spriteCacheCount].data = data;
        spriteCache[spriteCacheCount].width = width;
        spriteCache[spriteCacheCount].height = height;
        spriteCache[spriteCacheCount].bpp = bpp;
        spriteCache[spriteCacheCount].lastUsed = millis();
        spriteCacheCount++;
    }

    return data;
}

// ==================== Очистка кэша ====================
void Graphics::cleanupSpriteCache() {
    uint32_t oldestTime = UINT32_MAX;
    uint8_t oldestIndex = 0;

    // Поиск самого старого спрайта
    for (uint8_t i = 0; i < spriteCacheCount; i++) {
        if (spriteCache[i].lastUsed < oldestTime) {
            oldestTime = spriteCache[i].lastUsed;
            oldestIndex = i;
        }
    }

    // Освобождение памяти
    delete[] spriteCache[oldestIndex].data;

    // Сдвиг массива
    for (uint8_t i = oldestIndex; i < spriteCacheCount - 1; i++) {
        spriteCache[i] = spriteCache[i + 1];
    }

    spriteCacheCount--;
}

// ==================== Отрисовка игровых объектов ====================
void Graphics::drawShip(int16_t x, int16_t y, uint16_t angle, bool isBoosting, uint8_t boostFrame) {
    uint16_t normalizedAngle = (angle / 10) * 10;
    char filename[32];

    if (isBoosting) {
        snprintf(filename, sizeof(filename), "/sprites/ship/boost_%03d_%d.bin", normalizedAngle, boostFrame + 1);
    }
    else {
        snprintf(filename, sizeof(filename), "/sprites/ship/stay_%03d.bin", normalizedAngle);
    }

    const uint8_t* sprite = loadSprite(filename);
    if (sprite) {
        drawSprite(centerX(x, SHIP_WIDTH), centerY(y, SHIP_HEIGHT), sprite, SHIP_WIDTH, SHIP_HEIGHT, 4);
    }
}

void Graphics::drawBullet(int16_t x, int16_t y, uint16_t angle) {
    uint16_t normalizedAngle = (angle / 10) * 10;
    char filename[32];
    snprintf(filename, sizeof(filename), "/sprites/bullet/%03d.bin", normalizedAngle);

    const uint8_t* sprite = loadSprite(filename);
    if (sprite) {
        drawSprite(centerX(x, BULLET_WIDTH), centerY(y, BULLET_HEIGHT), sprite, BULLET_WIDTH, BULLET_HEIGHT, 1);
    }
}

void Graphics::drawComet(int16_t x, int16_t y, uint16_t direction) {
    uint16_t normalizedDirection = (direction / 10) * 10;
    char filename[32];
    snprintf(filename, sizeof(filename), "/sprites/comet/%03d.bin", normalizedDirection);

    const uint8_t* sprite = loadSprite(filename);
    if (sprite) {
        drawSprite(centerX(x, COMET_WIDTH), centerY(y, COMET_HEIGHT), sprite, COMET_WIDTH, COMET_HEIGHT, 4);
    }
}

// ==================== Основные экраны ====================
void Graphics::drawLogo() {
    const uint8_t* sprite = loadSprite("/sprites/logo.bin");
    if (sprite) {
        drawSprite(0, 0, sprite, LOGO_WIDTH, LOGO_HEIGHT, 4);
    }
}

void Graphics::drawStartMenu(uint8_t frame) {
    // Фон
    const uint8_t* bg = loadSprite("/sprites/bg_menu.bin");
    if (bg) {
        drawSprite(0, 0, bg, BG_WIDTH, BG_HEIGHT, 4);
    }

    // Анимированные элементы
    if (frame % 20 < 10) {
        const uint8_t* name1 = loadSprite("/sprites/name1.bin");
        if (name1) drawSprite(5, 20, name1, NAME_WIDTH, NAME_HEIGHT, 4);
    }
    else {
        const uint8_t* name2 = loadSprite("/sprites/name2.bin");
        if (name2) drawSprite(5, 20, name2, NAME_WIDTH, NAME_HEIGHT, 4);
    }

    if (frame % 10 < 5) {
        const uint8_t* press = loadSprite("/sprites/press.bin");
        if (press) drawSprite(40, 120, press, PRESS_WIDTH, PRESS_HEIGHT, 4);
    }
}

void Graphics::drawGameBackground() {
    const uint8_t* bg = loadSprite("/sprites/bg_game.bin");
    if (bg) {
        drawSprite(0, 0, bg, MAIN_BG_WIDTH, MAIN_BG_HEIGHT, 4);
    }
}

void Graphics::drawGameOver(uint16_t score, uint16_t highScore) {
    const uint8_t* bg = loadSprite("/sprites/bg_gameover.bin");
    if (bg) {
        drawSprite(0, 0, bg, BG_WIDTH, BG_HEIGHT, 4);
    }

    // Вывод счета
    drawScore(score);

    // Вывод рекорда
    char highScoreText[20];
    snprintf(highScoreText, sizeof(highScoreText), "BEST: %d", highScore);
    tft->setTextColor(TFT_YELLOW, TFT_BLACK);
    tft->drawString(highScoreText, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 20, 2);
}

// ==================== Утилиты ====================
void Graphics::drawScore(uint16_t score) {
    char scoreText[15];
    snprintf(scoreText, sizeof(scoreText), "SCORE: %d", score);

    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextDatum(TL_DATUM);
    tft->drawString(scoreText, 5, 5, 2);
}

bool Graphics::isSpriteVisible(int16_t x, int16_t y, uint8_t w, uint8_t h) {
    return !(x + w <= 0 || x >= SCREEN_WIDTH || y + h <= 0 || y >= SCREEN_HEIGHT);
}
