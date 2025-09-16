#include "storage.h"
#include <SD_MMC.h>

static const char* HIGH_FILE = "/highscore.txt";

// ==================== Инициализация ====================
bool storageInit() {
    if (!SD_MMC.begin()) {
        Serial.println("SD_MMC mount failed!");
        return false;
    }
    return true;
}

// ==================== Загрузка рекорда ====================
int StorageManager::readHighScore() {
    File file = SD.open("/highscore.txt");
    if (!file) return 0;

    int highScore = file.parseInt();
    file.close();
    return highScore;
}

// ==================== Сохранение рекорда ====================
void StorageManager::writeHighScore(int score) {
    File file = SD.open("/highscore.txt", FILE_WRITE);
    if (file) {
        file.println(score);
        file.close();
    }
}
