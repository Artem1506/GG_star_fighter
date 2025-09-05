#include <TFT_eSPI.h>
#include <SPI.h>
#include <SD_MMC.h>

// ==================== DISPLAY ====================
TFT_eSPI tft = TFT_eSPI();

// Размеры спрайтов
#define LOGO_WIDTH   128
#define LOGO_HEIGHT  160
#define BG_WIDTH     128
#define BG_HEIGHT    160
#define NAME_WIDTH   117
#define NAME_HEIGHT  48
#define PRESS_WIDTH 48
#define PRESS_HEIGHT 15
// Цвет прозрачности (ярко-розовый, RGB565)
#define TRANSPARENT_COLOR 0xF81F

// ==================== SD CARD INIT ====================
bool initSDCard() {
    Serial.println("[INFO] Initializing SD card...");
    if (!SD_MMC.begin("/sdcard", true)) { // 1-bit mode
        Serial.println("[WARN] SD Card Mount Failed");
        return false;
    }
    if (SD_MMC.cardType() == CARD_NONE) {
        Serial.println("[WARN] No SD_MMC card attached");
        return false;
    }
    return true;
}

// ==================== SHOW IMAGE FROM FILE ====================
// Вариант с colorkey-прозрачностью
void displayRGB565FileTransparent(const char* filename, int x, int y, int width, int height, uint16_t transparentColor) {
    File file = SD_MMC.open(filename, FILE_READ);
    if (!file) {
        Serial.printf("[WARN] Cannot open file: %s\n", filename);
        return;
    }

    size_t expectedSize = width * height * 2;
    if (file.size() != expectedSize) {
        Serial.printf("[WARN] Wrong file size: %d vs %d\n", file.size(), expectedSize);
        file.close();
        return;
    }

    const int BUFFER_SIZE = 64;
    uint16_t buffer[BUFFER_SIZE];

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col += BUFFER_SIZE) {
            int pixelsToRead = min(BUFFER_SIZE, width - col);
            size_t bytesRead = file.read((uint8_t*)buffer, pixelsToRead * 2);
            if (bytesRead == pixelsToRead * 2) {
                for (int i = 0; i < pixelsToRead; i++) {
                    if (buffer[i] != transparentColor) {
                        tft.drawPixel(x + col + i, y + row, buffer[i]);
                    }
                }
            }
        }
    }

    file.close();
}

// Обычная отрисовка без прозрачности
void displayRGB565File(const char* filename, int x, int y, int width, int height) {
    File file = SD_MMC.open(filename, FILE_READ);
    if (!file) return;

    const int BUFFER_SIZE = 64;
    uint16_t buffer[BUFFER_SIZE];

    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col += BUFFER_SIZE) {
            int pixelsToRead = min(BUFFER_SIZE, width - col);
            size_t bytesRead = file.read((uint8_t*)buffer, pixelsToRead * 2);
            if (bytesRead == pixelsToRead * 2) {
                tft.pushImage(x + col, y + row, pixelsToRead, 1, buffer);
            }
        }
    }

    file.close();
}

// ==================== SETUP ====================
void setup() {
    Serial.begin(115200);
    delay(500);

    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);

    if (!initSDCard()) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setCursor(10, 70);
        tft.println("SD init failed!");
        return;
    }

    // 1) Логотип 5 секунд
    displayRGB565File("/spr_GG_logo.bin", 0, 0, LOGO_WIDTH, LOGO_HEIGHT);
    delay(5000);

    // 2) Фон
    displayRGB565File("/spr_start_BG.bin", 0, 0, BG_WIDTH, BG_HEIGHT);
}

// ==================== LOOP ====================
void loop() {
    static unsigned long lastToggle = 0;
    static bool showAlt = false;

    static unsigned long lastPressToggle = 0;
    static bool showPress = false;

    unsigned long now = millis();

    // Переключение spr_main_name <-> spr_main_name2 каждые 1.5 секунды
    if (now - lastToggle >= 1500) {
        lastToggle = now;
        showAlt = !showAlt;

        if (showAlt) {
            displayRGB565FileTransparent("/spr_main_name.bin", 3, 109, NAME_WIDTH, NAME_HEIGHT, TRANSPARENT_COLOR);
        } else {
            displayRGB565FileTransparent("/spr_main_name2.bin", 3, 109, NAME_WIDTH, NAME_HEIGHT, TRANSPARENT_COLOR);
        }
    }

    // Мигание spr_press_RB.bin (1 сек есть, 1 сек нет)
    if (now - lastPressToggle >= 1000) {
        lastPressToggle = now;
        showPress = !showPress;

        if (showPress) {
            displayRGB565FileTransparent("/spr_press_RB.bin", 37, 71, PRESS_WIDTH, PRESS_HEIGHT, TRANSPARENT_COLOR);
        } else {
            // перерисуем фон на месте кнопки (чтобы стереть)
            displayRGB565File("/spr_start_BG.bin", 37, 71, PRESS_WIDTH, PRESS_HEIGHT);
        }
    }
}
