#include "graphics.h"
#include <SD_MMC.h>
#include <TFT_eSPI.h>

// ==================== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ====================
static TFT_eSPI* tft = nullptr;
static CachedSprite spriteCache[MAX_CACHED_SPRITES];
static uint8_t spriteCacheCount = 0;

// ==================== ИНИЦИАЛИЗАЦИЯ ====================
bool Graphics::init() {
    tft = new TFT_eSPI();
    tft->init();
    tft->setRotation(0);
    tft->fillScreen(TFT_BLACK);

    // ЕДИНАЯ ИНИЦИАЛИЗАЦИЯ SD ЧЕРЕЗ StorageManager
    if (!storage.init()) {
        Serial.println("Storage initialization failed!");
        return false;
    }

    // Предзагрузка критичных спрайтов
    preloadEssentialSprites();

    return true;
}

void Graphics::present() {
}

void Graphics::clear() {
    tft->fillScreen(TFT_BLACK);
}

// ==================== ПРЕДЗАГРУЗКА СПРАЙТОВ ====================
void Graphics::preloadEssentialSprites() {
    // ВСЕ ФАЙЛЫ В КОРНЕ SD-КАРТЫ
    const char* essentialSprites[] = {
        LOGO_FILE,
        START_BG_FILE,
        MAIN_BG_FILE,
        GAMEOVER_BG_FILE,
        NAME1_FILE,
        NAME2_FILE,
        PRESS_FILE
    };

    for (uint8_t i = 0; i < sizeof(essentialSprites) / sizeof(essentialSprites[0]); i++) {
        loadSprite(essentialSprites[i]);
    }
}

// ==================== ЗАГРУЗКА СПРАЙТОВ ====================
const uint8_t* Graphics::loadSprite(const char* filename) {
    // Поиск в кэше
    for (uint8_t i = 0; i < spriteCacheCount; i++) {
        if (strcmp(spriteCache[i].filename, filename) == 0) {
            spriteCache[i].lastUsed = millis();
            return spriteCache[i].data;
        }
    }

    // Освобождение места если нужно
    if (spriteCacheCount >= MAX_CACHED_SPRITES) {
        cleanupSpriteCache();
    }

    // Загрузка с SD (все файлы в корне)
    File file = storage.openFile(filename, FILE_READ);
    if (!file) {
        Serial.print("Failed to open: ");
        Serial.println(filename);
        return nullptr;
    }

    // Автоопределение параметров
    uint8_t width = 0, height = 0, bpp = 4;

    if (strstr(filename, "ship")) {
        width = SHIP_WIDTH; height = SHIP_HEIGHT;
    }
    else if (strstr(filename, "bullet")) {
        width = BULLET_WIDTH; height = BULLET_HEIGHT; bpp = 1;
    }
    else if (strstr(filename, "comet")) {
        width = COMET_WIDTH; height = COMET_HEIGHT;
    }
    else if (strstr(filename, "explosion")) {
        width = EXPLOSION_WIDTH; height = EXPLOSION_HEIGHT;
    }
    else if (strstr(filename, "asteroid")) {
        width = ASTEROID_WIDTH; height = ASTEROID_HEIGHT;
    }
    else if (strstr(filename, "spr_GG_logo")) {
        width = LOGO_WIDTH; height = LOGO_HEIGHT;
    }
    else if (strstr(filename, "spr_start_BG") || strstr(filename, "spr_main_BG") || strstr(filename, "spr_GO_BG")) {
        width = BG_WIDTH; height = BG_HEIGHT;
    }
    else if (strstr(filename, "spr_main_name")) {
        width = NAME_WIDTH; height = NAME_HEIGHT;
    }
    else if (strstr(filename, "spr_press_RB")) {
        width = PRESS_WIDTH; height = PRESS_HEIGHT;
    }

    size_t size = file.size();
    uint8_t* data = new uint8_t[size];
    file.read(data, size);
    file.close();

    // Сохранение в кэш
    if (spriteCacheCount < MAX_CACHED_SPRITES) {
        strncpy(spriteCache[spriteCacheCount].filename, filename, MAX_FILENAME_LENGTH - 1);
        spriteCache[spriteCacheCount].data = data;
        spriteCache[spriteCacheCount].width = width;
        spriteCache[spriteCacheCount].height = height;
        spriteCache[spriteCacheCount].bpp = bpp;
        spriteCache[spriteCacheCount].lastUsed = millis();
        spriteCacheCount++;
    }

    return data;
}

// ==================== ОЧИСТКА КЭША ====================
void Graphics::cleanupSpriteCache() {
    uint32_t oldestTime = UINT32_MAX;
    uint8_t oldestIndex = 0;

    for (uint8_t i = 0; i < spriteCacheCount; i++) {
        if (spriteCache[i].lastUsed < oldestTime) {
            oldestTime = spriteCache[i].lastUsed;
            oldestIndex = i;
        }
    }

    delete[] spriteCache[oldestIndex].data;

    for (uint8_t i = oldestIndex; i < spriteCacheCount - 1; i++) {
        spriteCache[i] = spriteCache[i + 1];
    }

    spriteCacheCount--;
}

// ==================== ОТРИСОВКА ИГРОВЫХ ОБЪЕКТОВ ====================
void Graphics::drawShip(int16_t x, int16_t y, uint16_t angle, bool isBoosting, uint8_t boostFrame) {
    char filename[28]; // Укороченные имена для корня
    uint16_t normalizedAngle = (angle / 10) * 10;

    if (isBoosting) {
        snprintf(filename, sizeof(filename), "/spr_ship_boost_%03d_%d.bin", normalizedAngle, boostFrame + 1);
    }
    else {
        snprintf(filename, sizeof(filename), "/spr_ship_stay_%03d.bin", normalizedAngle);
    }

    const uint8_t* sprite = loadSprite(filename);
    if (sprite) {
        drawSprite(centerX(x, SHIP_WIDTH), centerY(y, SHIP_HEIGHT), sprite, SHIP_WIDTH, SHIP_HEIGHT, 4);
    }
}

void Graphics::drawBullet(int16_t x, int16_t y, uint16_t angle) {
    char filename[24];
    uint16_t normalizedAngle = (angle / 10) * 10;
    snprintf(filename, sizeof(filename), "/spr_bullet_%03d.bin", normalizedAngle);

    const uint8_t* sprite = loadSprite(filename);
    if (sprite) {
        drawSprite(centerX(x, BULLET_WIDTH), centerY(y, BULLET_HEIGHT), sprite, BULLET_WIDTH, BULLET_HEIGHT, 1);
    }
}

void Graphics::drawComet(int16_t x, int16_t y, uint16_t direction) {
    char filename[24];
    uint16_t normalizedDirection = (direction / 10) * 10;
    snprintf(filename, sizeof(filename), "/spr_comet_%03d.bin", normalizedDirection);

    const uint8_t* sprite = loadSprite(filename);
    if (sprite) {
        drawSprite(centerX(x, COMET_WIDTH), centerY(y, COMET_HEIGHT), sprite, COMET_WIDTH, COMET_HEIGHT, 4);
    }
}

void Graphics::drawExplosion(int16_t x, int16_t y, uint8_t frame) {
    char filename[24];
    snprintf(filename, sizeof(filename), "/spr_explosion_%02d.bin", frame);

    const uint8_t* sprite = loadSprite(filename);
    if (sprite) {
        drawSprite(centerX(x, EXPLOSION_WIDTH), centerY(y, EXPLOSION_HEIGHT), sprite, EXPLOSION_WIDTH, EXPLOSION_HEIGHT, 4);
    }
}

void Graphics::drawAsteroid(int16_t x, int16_t y, uint8_t size) {
    char filename[24];
    snprintf(filename, sizeof(filename), "/spr_asteroid_%d.bin", size);

    const uint8_t* sprite = loadSprite(filename);
    if (sprite) {
        drawSprite(centerX(x, ASTEROID_WIDTH), centerY(y, ASTEROID_HEIGHT), sprite, ASTEROID_WIDTH, ASTEROID_HEIGHT, 4);
    }
}

// ==================== ОСНОВНЫЕ ЭКРАНЫ ====================
void Graphics::drawLogo() {
    const uint8_t* sprite = loadSprite(LOGO_FILE);
    if (sprite) {
        drawSprite(0, 0, sprite, LOGO_WIDTH, LOGO_HEIGHT, 4);
    }
}

void Graphics::drawStartMenu(uint8_t frame) {
    // Фон
    const uint8_t* bg = loadSprite(START_BG_FILE);
    if (bg) drawSprite(0, 0, bg, BG_WIDTH, BG_HEIGHT, 4);

    // Анимация названия (1 секунда)
    if ((frame / 60) % 2 == 0) {
        const uint8_t* name1 = loadSprite(NAME1_FILE);
        if (name1) drawSprite(5, 20, name1, NAME_WIDTH, NAME_HEIGHT, 4);
    }
    else {
        const uint8_t* name2 = loadSprite(NAME2_FILE);
        if (name2) drawSprite(5, 20, name2, NAME_WIDTH, NAME_HEIGHT, 4);
    }

    // Мигающая надпись (0.5 секунды)
    if ((frame / 30) % 2 == 0) {
        const uint8_t* press = loadSprite(PRESS_FILE);
        if (press) drawSprite(40, 120, press, PRESS_WIDTH, PRESS_HEIGHT, 4);
    }
}

void Graphics::drawGameBackground() {
    const uint8_t* bg = loadSprite(MAIN_BG_FILE);
    if (bg) {
        drawSprite(0, 0, bg, BG_WIDTH, BG_HEIGHT, 4);
    }
}

void Graphics::drawGameOver(uint16_t score, uint16_t highScore) {
    const uint8_t* bg = loadSprite(GAMEOVER_BG_FILE);
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

// ==================== УТИЛИТЫ ====================
void Graphics::drawScore(uint16_t score) {
    char scoreText[16];
    snprintf(scoreText, sizeof(scoreText), "SCORE: %d", score);
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->drawString(scoreText, 5, 5, 2);
}

bool Graphics::isSpriteVisible(int16_t x, int16_t y, uint8_t w, uint8_t h) {
    return !(x + w <= 0 || x >= SCREEN_WIDTH || y + h <= 0 || y >= SCREEN_HEIGHT);
}
