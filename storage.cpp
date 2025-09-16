#include "storage.h"
#include <SD_MMC.h>

static const char* HIGH_FILE = "/highscore.txt";

// ==================== ������������� ====================
bool storageInit() {
    if (!SD_MMC.begin()) {
        Serial.println("SD_MMC mount failed!");
        return false;
    }
    return true;
}

// ==================== �������� ������� ====================
unsigned long loadHighscore() {
    File f = SD_MMC.open(HIGH_FILE);
    if (!f) {
        Serial.println("No highscore file, starting from 0");
        return 0;
    }
    unsigned long hs = f.parseInt();
    f.close();
    return hs;
}

// ==================== ���������� ������� ====================
void saveHighscore(unsigned long score) {
    File f = SD_MMC.open(HIGH_FILE, FILE_WRITE);
    if (!f) {
        Serial.println("Failed to open highscore file for writing!");
        return;
    }
    f.println(score);
    f.close();
}
