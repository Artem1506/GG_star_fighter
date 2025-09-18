#include "storage.h"
#include <SD_MMC.h>

// ==================== ��������� ������ ====================
const char* StorageManager::HIGHSCORE_FILE = "/highscore.txt";

// ==================== ������ ������������� ====================
bool StorageManager::init() {
    // ������������� ������������� � graphics.cpp
    if (!SD_MMC.begin("/sdcard", true)) { // 1-bit �����
        Serial.println("SD_MMC initialization failed!");
        return false;
    }

    // �������� ������������� ����� �������
    if (!SD_MMC.exists(HIGHSCORE_FILE)) {
        File file = SD_MMC.open(HIGHSCORE_FILE, FILE_WRITE);
        if (file) {
            file.println("0");
            file.close();
        }
    }

    return true;
}

// ==================== ������ ������� ====================
int StorageManager::readHighScore() {
    File file = SD_MMC.open(HIGHSCORE_FILE, FILE_READ);
    if (!file) return 0;

    int highScore = file.parseInt();
    file.close();
    return highScore;
}

// ==================== ������ ������� ====================
void StorageManager::writeHighScore(int score) {
    File file = SD_MMC.open(HIGHSCORE_FILE, FILE_WRITE);
    if (file) {
        file.seek(0); // �������������� � ������
        file.println(score);
        file.close();
    }
}

// ==================== ������������� ������ � ������ ====================
File StorageManager::openFile(const char* filename, const char* mode) {
    return SD_MMC.open(filename, mode);
}
