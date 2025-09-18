#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <Arduino.h>
#include "storage.h"

// ==================== Константы экрана ====================
constexpr int16_t SCREEN_WIDTH = 128;
constexpr int16_t SCREEN_HEIGHT = 160;

// ==================== КОНСТАНТЫ ПУТЕЙ ФАЙЛОВ ====================
// ВСЕ ФАЙЛЫ В КОРНЕ SD-КАРТЫ
constexpr const char* LOGO_FILE = "/spr_GG_logo.bin";
constexpr const char* START_BG_FILE = "/spr_start_BG.bin";
constexpr const char* MAIN_BG_FILE = "/spr_main_BG.bin";
constexpr const char* GAMEOVER_BG_FILE = "/spr_GO_BG.bin";
constexpr const char* NAME1_FILE = "/spr_main_name.bin";
constexpr const char* NAME2_FILE = "/spr_main_name2.bin";
constexpr const char* PRESS_FILE = "/spr_press_RB.bin";

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

// ==================== КОНСТАНТЫ КЭШИРОВАНИЯ ====================
constexpr uint8_t MAX_CACHED_SPRITES = 15;
constexpr uint8_t MAX_FILENAME_LENGTH = 24;

// ==================== СТРУКТУРА КЭША ====================
struct CachedSprite {
    char filename[MAX_FILENAME_LENGTH];
    uint8_t* data;
    uint8_t width;
    uint8_t height;
    uint8_t bpp;
    uint32_t lastUsed;
};

// ==================== ОСНОВНОЙ КЛАСС ====================
class Graphics {
public:
    bool init();
    void present();
    void clear();

    // Основные экраны согласно ТЗ
    void drawLogo();
    void drawStartMenu(uint8_t frame);
    void drawGameBackground();
    void drawGameOver(uint16_t score, uint16_t highScore);

    // Игровые объекты согласно ТЗ
    void drawShip(int16_t x, int16_t y, uint16_t angle, bool isBoosting = false, uint8_t boostFrame = 0);
    void drawBullet(int16_t x, int16_t y, uint16_t angle);
    void drawAsteroid(int16_t x, int16_t y, uint8_t size);
    void drawComet(int16_t x, int16_t y, uint16_t direction);
    void drawExplosion(int16_t x, int16_t y, uint8_t frame);
    void drawScore(uint16_t score);

    // Утилиты
    static int16_t centerX(int16_t x, uint8_t width) { return x - width / 2; }
    static int16_t centerY(int16_t y, uint8_t height) { return y - height / 2; }

private:
    StorageManager storage;
    void drawSprite(int16_t x, int16_t y, const uint8_t* bitmap, uint8_t w, uint8_t h, uint8_t bpp = 1);
    const uint8_t* loadSprite(const char* filename);
    void preloadEssentialSprites();
    void cleanupSpriteCache();
    bool isSpriteVisible(int16_t x, int16_t y, uint8_t w, uint8_t h);
};

#endif
