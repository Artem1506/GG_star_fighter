#include "storage.h"
#include <SD_MMC.h>

// ==================== КОНСТАНТЫ ФАЙЛОВ ====================
const char* StorageManager::HIGHSCORE_FILE = "/highscore.txt";

// ==================== ЕДИНАЯ ИНИЦИАЛИЗАЦИЯ ====================
bool StorageManager::init() {
    // ЕДИНООБРАЗНАЯ ИНИЦИАЛИЗАЦИЯ С graphics.cpp
    if (!SD_MMC.begin("/sdcard", true)) { // 1-bit режим
        Serial.println("SD_MMC initialization failed!");
        return false;
    }

    // Проверка существования файла рекорда
    if (!SD_MMC.exists(HIGHSCORE_FILE)) {
        File file = SD_MMC.open(HIGHSCORE_FILE, FILE_WRITE);
        if (file) {
            file.println("0");
            file.close();
        }
    }

    return true;
}

// ==================== ЧТЕНИЕ РЕКОРДА ====================
int StorageManager::readHighScore() {
    File file = SD_MMC.open(HIGHSCORE_FILE, FILE_READ);
    if (!file) return 0;

    int highScore = file.parseInt();
    file.close();
    return highScore;
}

// ==================== ЗАПИСЬ РЕКОРДА ====================
void StorageManager::writeHighScore(int score) {
    File file = SD_MMC.open(HIGHSCORE_FILE, FILE_WRITE);
    if (file) {
        file.seek(0); // Перезаписываем с начала
        file.println(score);
        file.close();
    }
}

// ==================== УНИВЕРСАЛЬНЫЙ ДОСТУП К ФАЙЛАМ ====================
File StorageManager::openFile(const char* filename, const char* mode) {
    return SD_MMC.open(filename, mode);
}
